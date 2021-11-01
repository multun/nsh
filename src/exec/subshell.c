#include <nsh_exec/ast_exec.h>
#include <nsh_utils/hashmap.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_exec/runtime_error.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

int subshell_exec(struct environment *env, struct shast *ast,
                  struct exception_catcher *catcher)
{
    struct shast_subshell *subshell = (struct shast_subshell *)ast;

    int cpid = managed_fork(env);
    if (cpid == -1) {
        shwarn(&subshell->base.line_info, "fork() failed: %s", strerror(errno));
        runtime_error(catcher, 1);
    }

    if (cpid == 0)
        clean_exit(catcher, ast_exec(env, subshell->action, catcher));


    int status;
    waitpid(cpid, &status, 0);
    return WEXITSTATUS(status);
}
