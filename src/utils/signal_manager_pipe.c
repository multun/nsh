#include "utils/signal_lut.h"
#include "utils/signal_manager.h"
#include "utils/signal_pipe.h"

#include <err.h>
#include <string.h>


static int signal_manager_pipe_read(struct signal_manager *sigman, struct signal_list *events);
static int signal_manager_pipe_signal_enabled(struct signal_manager *sigman, int signal);
static int signal_manager_pipe_signal_disabled(struct signal_manager *sigman, int signal);
static int signal_manager_pipe_pre_fork_hook(struct signal_manager *sigman);
static int signal_manager_pipe_post_fork_hook(struct signal_manager *sigman, pid_t pid);
static int signal_manager_pipe_fd(struct signal_manager *sigman __unused);


static struct signal_manager_type signal_manager_pipe_type = {
    .read = signal_manager_pipe_read,
    .signal_enabled = signal_manager_pipe_signal_enabled,
    .signal_disabled = signal_manager_pipe_signal_disabled,
    .pre_fork_hook = signal_manager_pipe_pre_fork_hook,
    .post_fork_hook = signal_manager_pipe_post_fork_hook,
    .fd = signal_manager_pipe_fd,
};


int signal_manager_pipe_init(struct signal_manager *sigman)
{
    signal_manager_init(sigman, &signal_manager_pipe_type);
    if (signal_pipe_init() < 0)
        return -1;
    return 0;
}

static int signal_manager_pipe_read(struct signal_manager *sigman __unused, struct signal_list *events __unused)
{
    if (signal_pipe_read(events) == -1)
        return -1;

    signal_lut_read(events);
    return 0;
}

static int signal_manager_pipe_signal_enabled(struct signal_manager *sigman __unused, int signal)
{
    int rc;
    if ((rc = signal_lut_setup_handler(&sigman->saved_handlers[signal], signal, SA_RESTART, signal_pipe_lut_handler))) {
        warn("failed to setup the %s handler", strsignal(signal));
        return -1;
    }
    return 0;
}

static int signal_manager_pipe_signal_disabled(struct signal_manager *sigman __unused, int signal)
{
    int rc;
    if ((rc = sigaction(signal, &sigman->saved_handlers[signal], NULL)) == -1) {
        warn("failed to restore the %s handler", strsignal(signal));
        return -1;
    }
    return 0;
}

static int signal_manager_pipe_pre_fork_hook(struct signal_manager *sigman __unused)
{
    return signal_pipe_pre_fork();
}

static int signal_manager_pipe_post_fork_hook(struct signal_manager *sigman __unused, pid_t pid)
{
    return signal_pipe_post_fork(pid);
}

static int signal_manager_pipe_fd(struct signal_manager *sigman __unused)
{
    return signal_pipe_fd();
}
