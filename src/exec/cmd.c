#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_exec/expansion.h>
#include <nsh_utils/hashmap.h>
#include <nsh_utils/macros.h>


#include "args.h"
#include "config.h"


// this function is here to free the expanded array in case the expansion fails
static void wordlist_expand(struct cpvect *res, struct wordlist *wl,
                            struct environment *env, struct exception_catcher *catcher)
{
    cpvect_init(res, wordlist_size(wl) + 1);

    /* on exception, free the result array */
    struct exception_catcher sub_catcher = EXCEPTION_CATCHER(catcher->context, catcher);
    if (setjmp(sub_catcher.env)) {
        /* free the result vector elements */
        for (size_t i = 0; i < cpvect_size(res); i++)
            free(cpvect_get(res, i));
        cpvect_destroy(res);
        shraise(catcher, NULL);
    }

    /* expand the argument list */
    expand_wordlist(res, wl, 0, env, &sub_catcher);
}


static int builtin_exec(struct environment *env, struct exception_catcher *catcher,
                        f_builtin builtin)
{
    int res = builtin(env, catcher, env->argc, env->argv);
    fflush(stdout);
    return res;
}

static int cmd_fork_exec(struct environment *env, struct exception_catcher *catcher)
{
    int status;
    pid_t pid = managed_fork(env);
    if (pid < 0)
        clean_err(env, catcher, errno, "cmd_exec: error while forking");

    /* parent branch */
    if (pid != 0) {
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

    /* child branch */
    char **penv = environment_array(env);
    execvpe(env->argv[0], env->argv, penv);

    /*on failure, free the environment array */
    for (size_t i = 0; penv[i]; i++)
        free(penv[i]);
    free(penv);

    clean_err(env, catcher, 125 + errno, "couldn't exec \"%s\"", env->argv[0]);
}

static void function_call_cleanup(struct environment *env, struct shast_function *func)
{
    if (func == NULL)
        return;

    assert(env->call_depth);
    env->call_depth--;
    shast_function_put(func);
}


static int cmd_run_command(struct environment *env, struct exception_catcher *catcher,
                           struct shast_function *volatile *func)
{
    /* look for functions */
    struct hashmap_item *func_hash = hashmap_find(&env->functions, NULL, env->argv[0]);
    if (func_hash) {
        *func = container_of(func_hash, struct shast_function, hash);
        shast_function_get(*func);
        env->call_depth++;

        if (env->call_depth >= MAX_CALL_DEPTH) {
            warnx("maximum call depth of %d reached", MAX_CALL_DEPTH);
            runtime_error(env, catcher, 1);
        }

        return ast_exec(env, (*func)->body, catcher);
    }

    /* look for builtins */
    f_builtin builtin = env->find_builtin(env->argv[0]);
    if (builtin)
        return builtin_exec(env, catcher, builtin);
    /* no function or builtin found, fork and exec */
    return cmd_fork_exec(env, catcher);
}

int cmd_exec(struct environment *env, struct shast *ast,
             struct exception_catcher *catcher)
{
    struct shast_cmd *command = (struct shast_cmd *)ast;
    struct wordlist *wl = &command->arguments;
    int volatile prev_argc = env->argc;
    char **volatile prev_argv = env->argv;

    /* expand the arguments array */
    struct cpvect new_argv;
    wordlist_expand(&new_argv, wl, env, catcher);

    /* when the array didn't expand to anything, there's no command to run */
    if (cpvect_size(&new_argv) == 0) {
        argv_free(cpvect_size(&new_argv), cpvect_data(&new_argv));
        return 0;
    }

    /* setup argc / argv using the expanded array */
    env->argc = cpvect_size(&new_argv);
    cpvect_push(&new_argv, NULL);
    env->argv = cpvect_data(&new_argv);

    /* if we're running a function, the refcnt has to be dropped on exit */
    struct shast_function *volatile func = NULL;

    /* on exception, free the argument array */
    struct exception_catcher sub_catcher = EXCEPTION_CATCHER(catcher->context, catcher);
    if (setjmp(sub_catcher.env)) {
        function_call_cleanup(env, func);
        argv_free(env->argc, env->argv);
        env->argc = prev_argc;
        env->argv = prev_argv;
        shraise(catcher, NULL);
    }

    /* run the command */
    int rc = cmd_run_command(env, &sub_catcher, &func);

    /* cleanup */
    function_call_cleanup(env, func);
    argv_free(env->argc, env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return rc;
}
