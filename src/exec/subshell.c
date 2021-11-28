#include <nsh_exec/ast_exec.h>
#include <nsh_utils/hashmap.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/managed_fork.h>
#include <nsh_exec/runtime_error.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

nsh_err_t subshell_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_subshell *subshell = (struct shast_subshell *)ast;

    int cpid = managed_fork(env);
    if (cpid == -1) {
        lineinfo_warn(&subshell->base.line_info, "fork() failed: %s", strerror(errno));
        return execution_error(env, 1);
    }

    if (cpid == 0) {
        if ((err = ast_exec(env, subshell->action)))
            return err;
        return clean_exit(env, err);
    }

    int status;
    waitpid(cpid, &status, 0);
    env->code = WEXITSTATUS(status);
    return NSH_OK;
}
