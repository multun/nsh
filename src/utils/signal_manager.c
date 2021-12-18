#include <nsh_utils/signal_manager.h>
#include <nsh_utils/signal_list.h>
#include <nsh_utils/macros.h>

#include <unistd.h>

void signal_manager_init(struct signal_manager *sigman)
{
    for (size_t i = 0; i < ARR_SIZE(sigman->signal_handlers); i++)
        list_init(&sigman->signal_handlers[i]);

    sigman->signal_enabled = NULL;
    sigman->signal_disabled = NULL;
    sigman->pre_fork_hook = NULL;
    sigman->post_fork_hook = NULL;
    sigman->hook_state = NULL;
}

static struct list_head *handler_list_get(struct signal_manager *sigman, int signal)
{
    assert(signal < MAX_SIGNAL_NUMBER);
    return &sigman->signal_handlers[signal];
}

void signal_manager_setup_handler(struct signal_manager *sigman,
                                  struct signal_handler *handler, int signal,
                                  bool head_handler)
{
    struct list_head *handler_list = handler_list_get(sigman, signal);
    handler->signal = signal;
    handler->sigman = sigman;
    (head_handler ? list_add : list_add_tail)(&handler->__list, handler_list);

    if (list_single(handler_list))
        sigman->signal_enabled(sigman->hook_state, signal);
}

int signal_manager_dispatch(struct signal_manager *sigman, int signal)
{
    int rc;
    struct list_head *handler_list = handler_list_get(sigman, signal);

    struct signal_handler *handler;
    struct signal_handler *tmp;
    list_for_each_entry_safe(handler, tmp, handler_list, struct signal_handler, __list)
    {
        if ((rc = handler->handle(handler)))
            return rc;
    }
    return 0;
}

void signal_handler_del(struct signal_handler *handler)
{
    list_del(&handler->__list);
    int signal = handler->signal;
    struct signal_manager *sigman = handler->sigman;
    if (list_empty(handler_list_get(sigman, signal)))
        sigman->signal_disabled(sigman->hook_state, signal);
}

pid_t signal_manager_fork(struct signal_manager *sigman)
{
    if (sigman->pre_fork_hook)
        sigman->pre_fork_hook(sigman->hook_state);

    pid_t res = fork();

    if (sigman->post_fork_hook)
        sigman->post_fork_hook(sigman->hook_state, res);

    return res;
}
