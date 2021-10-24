#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <nsh_exec/ast_exec.h>
#include <nsh_exec/args.h>
#include <nsh_exec/config.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_exec/environment.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_utils/hash_table.h>
#include <nsh_utils/macros.h>

static int builtin_exec(struct environment *env, struct ex_scope *ex_scope, f_builtin builtin)
{
    int res = builtin(env, ex_scope, env->argc, env->argv);
    fflush(stdout);
    return res;
}

static int cmd_fork_exec(struct environment *env, struct ex_scope *ex_scope)
{
    int status;
    pid_t pid = managed_fork(env);
    if (pid < 0)
        clean_err(ex_scope, errno, "cmd_exec: error while forking");

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

    clean_err(ex_scope, 125 + errno, "couldn't exec \"%s\"", env->argv[0]);
}

static void function_call_cleanup(struct environment *env, struct shast_function *func)
{
    if (func == NULL)
        return;

    assert(env->call_depth);
    env->call_depth--;
    shast_function_put(func);
}


static int cmd_run_command(struct environment *env, struct ex_scope *ex_scope, struct shast_function * volatile *func)
{
    /* look for functions */
    struct hash_head *func_hash = hash_table_find(&env->functions, NULL, env->argv[0]);
    if (func_hash)
    {
        *func = container_of(func_hash, struct shast_function, hash);
        shast_function_get(*func);
        env->call_depth++;

        if (env->call_depth >= MAX_CALL_DEPTH) {
            warnx("maximum call depth of %d reached", MAX_CALL_DEPTH);
            runtime_error(ex_scope, 1);
        }

        return ast_exec(env, (*func)->body, ex_scope);
    }

    /* look for builtins */
    f_builtin builtin = env->find_builtin(env->argv[0]);
    if (builtin)
        return builtin_exec(env, ex_scope, builtin);
    /* no function or builtin found, fork and exec */
    return cmd_fork_exec(env, ex_scope);
}

int cmd_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    struct wordlist *wl = &command->arguments;
    int volatile prev_argc = env->argc;
    char **volatile prev_argv = env->argv;

    /* expand the arguments array */
    struct cpvect new_argv;
    wordlist_expand(&new_argv, wl, env, ex_scope);

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
    struct shast_function * volatile func = NULL;

    /* on exception, free the argument array */
    struct ex_scope sub_ex_scope = EXCEPTION_SCOPE(ex_scope->context, ex_scope);
    if (setjmp(sub_ex_scope.env)) {
        function_call_cleanup(env, func);
        argv_free(env->argc, env->argv);
        env->argc = prev_argc;
        env->argv = prev_argv;
        shraise(ex_scope, NULL);
    }

    /* run the command */
    int rc = cmd_run_command(env, &sub_ex_scope, &func);

    /* cleanup */
    function_call_cleanup(env, func);
    argv_free(env->argc, env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return rc;
}
