#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "shexec/args.h"
#include "shexec/builtins.h"
#include "shexec/clean_exit.h"
#include "shexec/environment.h"
#include "utils/hash_table.h"

void cmd_print(FILE *f, struct ast *node)
{
    void *id = node;
    struct wordlist *wl = node->data.ast_cmd.wordlist;
    fprintf(f, "\"%p\" [label=\"CMD\\n%s", id, wl->str);
    wl = wl->next;
    while (wl) {
        fprintf(f, " %s", wl->str);
        wl = wl->next;
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
    struct pair *p = htable_access(env->functions, env->argv[0]);
    if (p)
        return ast_exec(env, p->value, cont);

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

int cmd_exec(struct environment *env, struct ast *node, struct errcont *cont)
{
    struct wordlist *wl = node->data.ast_cmd.wordlist;
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
    } else {
        struct errcont ncont = ERRCONT(cont->errman, &keeper);
        wordlist_to_argv(&env->argv, wl, env, &ncont);
        res = cmd_exec_argv(env, &ncont);
    }

    argv_free(env->argv);
    env->argc = prev_argc;
    env->argv = prev_argv;
    return res;
}

void cmd_free(struct ast *ast)
{
    if (!ast)
        return;
    wordlist_free(ast->data.ast_cmd.wordlist, true);
    free(ast);
}
