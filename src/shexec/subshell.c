#include "shparse/ast.h"
#include "utils/hash_table.h"
#include "shexec/clean_exit.h"

#include <err.h>
#include <stdio.h>
#include <sys/wait.h>

int subshell_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    // TODO: error handling
    struct shast_subshell *subshell = (struct shast_subshell *)ast;

    int cpid = managed_fork(&env->sigman);
    if (cpid == -1)
        err(1, "fork() failed");

    if (cpid == 0)
        clean_exit(cont, ast_exec(env, subshell->action, cont));

    int status;
    waitpid(cpid, &status, 0);
    return WEXITSTATUS(status);
}
