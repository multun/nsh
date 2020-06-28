#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shparse/ast.h"
#include "shexec/args.h"
#include "shexec/builtins.h"
#include "shexec/clean_exit.h"
#include "shexec/environment.h"
#include "shexec/managed_fork.h"
#include "utils/hash_table.h"
#include "utils/macros.h"

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

static int cmd_run_command(struct environment *env, struct ex_scope *ex_scope, struct shast_function * volatile *func)
{
    /* look for functions */
    struct hash_head *func_hash = hash_table_find(&env->functions, NULL, env->argv[0]);
    if (func_hash)
    {
        *func = container_of(func_hash, struct shast_function, hash);
        shast_function_get(*func);
        return ast_exec(env, (*func)->body, ex_scope);
    }

    /* look for builtins */
    f_builtin builtin = builtin_search(env->argv[0]);
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
        argv_free(env->argc, env->argv);
        env->argc = prev_argc;
        env->argv = prev_argv;
        shast_function_put(func);
        shraise(ex_scope, NULL);
    }

    /* run the command */
    int rc = cmd_run_command(env, &sub_ex_scope, &func);

    /* cleanup */
    shast_function_put(func);
    argv_free(env->argc, env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return rc;
}
