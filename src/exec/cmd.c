#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_exec/expansion.h>
#include <nsh_utils/hashmap.h>
#include <nsh_utils/macros.h>


#include "execution_error.h"
#include "args.h"
#include "config.h"
#include "proc_utils.h"


// this function is here to free the expanded array in case the expansion fails
static nsh_err_t wordlist_expand(struct cpvect *res, struct wordlist *wl,
                                 struct environment *env)
{
    nsh_err_t err;
    cpvect_init(res, wordlist_size(wl) + 1);

    /* expand the argument list */
    if ((err = expand_wordlist(res, wl, env, 0)))
        goto expansion_error;
    return NSH_OK;

expansion_error:
    /* free the result vector elements */
    for (size_t i = 0; i < cpvect_size(res); i++)
        free(cpvect_get(res, i));
    cpvect_destroy(res);
    return err;
}


static nsh_err_t builtin_exec(struct environment *env, f_builtin builtin)
{
    nsh_err_t res = builtin(env, env->argc, env->argv);
    fflush(stdout);
    return res;
}

static nsh_err_t cmd_fork_exec(struct environment *env)
{
    pid_t pid = managed_fork(env);
    if (pid < 0)
        return clean_err(env, errno, "error while forking");

    /* parent branch */
    if (pid != 0) {
        int child_status = proc_wait_exit(pid);
        if (child_status < 0)
            return clean_err(
                env, errno,
                "error when waiting for the command child process to complete");
        env->code = child_status;
        return NSH_OK;
    }

    /* child branch */
    char **penv = environment_array(env);
    execvpe(env->argv[0], env->argv, penv);

    /* when the execution fail, exit 127 if the program does not exist, 126 otherwise */
    int exit_status = 126;
    if (errno == ENOENT)
        exit_status = 127;

    /* on failure, free the environment array */
    for (size_t i = 0; penv[i]; i++)
        free(penv[i]);
    free(penv);

    return clean_err(env, exit_status, "couldn't exec \"%s\"", env->argv[0]);
}

static nsh_err_t cmd_run_command(struct environment *env)
{
    /* look for functions */
    struct hashmap_item *func_hash = hashmap_find(&env->functions, NULL, env->argv[0]);
    if (func_hash) {
        struct shast_function *func =
            container_of(func_hash, struct shast_function, hash);
        shast_function_get(func);
        env->call_depth++;

        if (env->call_depth >= MAX_CALL_DEPTH) {
            warnx("maximum call depth of %d reached", MAX_CALL_DEPTH);
            return execution_error(env, 1);
        }

        nsh_err_t err = ast_exec(env, func->body);
        assert(env->call_depth);
        env->call_depth--;
        shast_function_put(func);
        return err;
    }

    /* look for builtins */
    f_builtin builtin = env->find_builtin(env->argv[0]);
    if (builtin)
        return builtin_exec(env, builtin);

    /* no function or builtin found, fork and exec */
    return cmd_fork_exec(env);
}

nsh_err_t cmd_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_cmd *command = (struct shast_cmd *)ast;
    struct wordlist *wl = &command->arguments;
    int prev_argc = env->argc;
    char **prev_argv = env->argv;

    /* expand the arguments array */
    struct cpvect new_argv;
    if ((err = wordlist_expand(&new_argv, wl, env)))
        return err;

    /* when the array didn't expand to anything, there's no command to run */
    if (cpvect_size(&new_argv) == 0) {
        argv_free(cpvect_size(&new_argv), cpvect_data(&new_argv));
        env->code = 0;
        return NSH_OK;
    }

    /* setup argc / argv using the expanded array */
    env->argc = cpvect_size(&new_argv);
    cpvect_push(&new_argv, NULL);
    env->argv = cpvect_data(&new_argv);

    /* run the command */
    err = cmd_run_command(env);

    /* cleanup */
    argv_free(env->argc, env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return err;
}
