#include "shexec/managed_fork.h"
#include "utils/signal_manager.h"


pid_t managed_fork(struct environment *env)
{
    pid_t res = signal_manager_fork(&env->sigman);
    if (res == 0)
        env->forked = true;
    return res;
}
