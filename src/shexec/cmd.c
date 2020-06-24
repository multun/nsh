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

static int builtin_exec(struct environment *env, struct errcont *cont, f_builtin builtin)
{
    int res = builtin(env, cont, env->argc, env->argv);
    fflush(stdout);
    return res;
}

static int cmd_fork_exec(struct environment *env, struct errcont *cont)
{
    int status;
    pid_t pid = managed_fork(env);
    if (pid < 0)
        clean_err(cont, errno, "cmd_exec: error while forking");

    /* parent branch */
    if (pid != 0) {
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

    /* child branch */
    char **penv = environment_array(env);
    execvpe(env->argv[0], env->argv, penv);
    argv_free(penv);
    clean_err(cont, 125 + errno, "couldn't exec \"%s\"", env->argv[0]);
}

static int cmd_run_command(struct environment *env, struct errcont *cont)
{
    /* look for functions */
    struct hash_head *func_hash = hash_table_find(&env->functions, NULL, env->argv[0]);
    if (func_hash)
    {
        struct shast_function *func = container_of(func_hash, struct shast_function, hash);
        return ast_exec(env, func->body, cont);
    }

    /* look for builtins */
    f_builtin builtin = builtin_search(env->argv[0]);
    if (builtin)
        return builtin_exec(env, cont, builtin);

    /* no function or builtin found, fork and exec */
    return cmd_fork_exec(env, cont);
}

int cmd_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    struct wordlist *wl = &command->arguments;
    int volatile prev_argc = env->argc;
    char **volatile prev_argv = env->argv;

    /* expand the arguments array */
    struct cpvect new_argv;
    wordlist_expand(&new_argv, wl, env, cont);

    /* setup argc / argv using the expanded array */
    env->argc = cpvect_size(&new_argv);
    cpvect_push(&new_argv, NULL);
    env->argv = cpvect_data(&new_argv);

    /* on exception, free the argument array */
    int res = 0;
    struct errcont sub_errcont = ERRCONT(cont->errman, cont);
    if (setjmp(sub_errcont.env)) {
        argv_free(env->argv);
        env->argc = prev_argc;
        env->argv = prev_argv;
        shraise(cont, NULL);
    }

    /* run the command */
    res = cmd_run_command(env, &sub_errcont);

    /* cleanup */
    argv_free(env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return res;
}
