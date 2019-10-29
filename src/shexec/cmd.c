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
#include "utils/hash_table.h"
#include "utils/macros.h"

void cmd_print(FILE *f, struct shast *ast)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    struct wordlist *wl = &command->arguments;
    fprintf(f, "\"%p\" [label=\"CMD\\n", (void*)ast);
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        if (i > 0)
            fputc(' ', f);
        fprintf(f, "%s", wordlist_get_str(wl, i));
    }
    fprintf(f, "\"];\n");
}

static int builtin_exec(struct environment *env, struct errcont *cont, f_builtin builtin)
{
    int res = builtin(env, cont, env->argc, env->argv);
    fflush(stdout);
    return res;
}

static int cmd_fork_exec(struct environment *env, struct errcont *cont)
{
    int status;
    pid_t pid = fork();
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
    struct keeper keeper = KEEPER(cont->keeper);

    /* expand the arguments array */
    env->argc = wordlist_to_argv(&env->argv, wl, env, cont);

    /* on exception, free the argument array */
    int res = 0;
    if (setjmp(keeper.env)) {
        argv_free(env->argv);
        env->argc = prev_argc;
        env->argv = prev_argv;
        shraise(cont, NULL);
    }

    /* run the command */
    res = cmd_run_command(env, &ERRCONT(cont->errman, &keeper));

    /* cleanup */
    argv_free(env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return res;
}

void cmd_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_cmd *command = (struct shast_cmd*)ast;
    wordlist_destroy(&command->arguments);
    free(command);
}
