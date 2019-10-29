#include "shparse/ast.h"
#include "utils/hash_table.h"
#include "shexec/clean_exit.h"

#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void subshell_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_subshell *subshell = (struct shast_subshell *)ast;
    ast_free(subshell->action);
    free(ast);
}

int subshell_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    // TODO: error handling
    struct shast_subshell *subshell = (struct shast_subshell *)ast;

    int cpid = fork();
    if (cpid == -1)
        err(1, "fork() failed");

    if (cpid == 0)
        clean_exit(cont, ast_exec(env, subshell->action, cont));

    int status;
    waitpid(cpid, &status, 0);
    return WEXITSTATUS(status);
}
