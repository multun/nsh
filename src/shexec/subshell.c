#include "shparse/ast.h"
#include "utils/hash_table.h"
#include "shexec/clean_exit.h"
#include "shexec/managed_fork.h"
#include "shexec/runtime_error.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

int subshell_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_subshell *subshell = (struct shast_subshell *)ast;

    int cpid = managed_fork(env);
    if (cpid == -1) {
        shwarn(&subshell->base.line_info, "fork() failed: %s", strerror(errno));
        runtime_error(ex_scope, 1);
    }

    if (cpid == 0)
        clean_exit(ex_scope, ast_exec(env, subshell->action, ex_scope));


    int status;
    waitpid(cpid, &status, 0);
    return WEXITSTATUS(status);
}
