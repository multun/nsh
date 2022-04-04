#include <nsh_exec/ast_exec.h>
#include <nsh_utils/hashmap.h>
#include <nsh_exec/clean_exit.h>
#include <nsh_exec/managed_fork.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>

#include "execution_error.h"
#include "proc_utils.h"


nsh_err_t subshell_exec(struct environment *env, struct shast *ast)
{
    nsh_err_t err;
    struct shast_subshell *subshell = (struct shast_subshell *)ast;

    pid_t child_pid = managed_fork(env);
    if (child_pid == -1) {
        lineinfo_warn(&subshell->base.line_info, "fork() failed: %s", strerror(errno));
        return execution_error(env, 1);
    }

    if (child_pid == 0) {
        if ((err = ast_exec(env, subshell->action)))
            return err;
        return clean_exit(env, err);
    }

    int child_status = proc_wait_exit(child_pid);
    if (child_status < 0)
        return clean_err(env, errno,
                         "error when waiting for the subshell child process to complete");
    env->code = child_status;
    return NSH_OK;
}
