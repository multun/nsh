#include "proc_utils.h"
#include <stdbool.h>
#include <assert.h>


int proc_wait_exit(pid_t pid)
{
    int status;
    while (true) {
        pid_t res_pid = waitpid(pid, &status, 0);
        if (res_pid == -1)
            return res_pid;
        assert(res_pid == pid);

        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);
    }
}
