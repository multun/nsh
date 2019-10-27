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
        fprintf(f, "%s", wordlist_get(wl, i));
    }
    fprintf(f, "\"];\n");
}

static int builtin_exec(struct environment *env, struct errcont *cont, f_builtin builtin)
{
    int res = builtin(env, cont, argv_count(env->argv), env->argv);
    fflush(stdout);
    return res;
}

static int cmd_exec_argv(struct environment *env, struct errcont *cont)
{
    struct hash_head *func_hash = hash_table_find(&env->functions, NULL, env->argv[0]);
    if (func_hash)
    {
        struct shast_function *func = container_of(func_hash, struct shast_function, hash);
        return ast_exec(env, func->body, cont);
    }

    f_builtin builtin = builtin_search(env->argv[0]);
    if (builtin)
        return builtin_exec(env, cont, builtin);

    int status;
    pid_t pid = fork();

    if (pid < 0)
        clean_err(cont, errno, "cmd_exec: error while forking");
    else if (pid == 0) {
        char **penv = environment_array(env);
        execvpe(env->argv[0], env->argv, penv);
        argv_free(penv);
        clean_err(cont, 125 + errno, "couldn't exec \"%s\"", env->argv[0]);
    }
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

int cmd_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    struct wordlist *wl = &command->arguments;
    int volatile prev_argc = env->argc;
    char **volatile prev_argv = env->argv;
    struct keeper keeper = KEEPER(cont->keeper);

    int res = 0;
    if (setjmp(keeper.env)) {
        if (prev_argv != env->argv) {
            argv_free(env->argv);
            env->argc = prev_argc;
            env->argv = prev_argv;
        }
        shraise(cont, NULL);
    }

    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    wordlist_to_argv(&env->argv, wl, env, &ncont);
    res = cmd_exec_argv(env, &ncont);
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
