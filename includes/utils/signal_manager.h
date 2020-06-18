#pragma once

#include "utils/list.h"
#include "utils/error.h"
#include "utils/signal_list.h"

#include <stdbool.h>

/**
** \page Signals
**
** The following component need to receive signals:
**  - the process manager with SIGCHLD
**  - the io backend and SIGINT
**  - the runtime and SIGINT
**  - trap and any signal
**
** The signal manager handles dispatching signals to these components, but doesn't handle
** interfacing with the operating system.
**
** When a signal is received, it must be dispatched using signal_manager_dispatch.
** When a new signal is expected by the application, the signal_enabled callback is called.
** When a signal is not expected anymore by the application, the signal_disabled callback is called.
*/

typedef void (*signal_manager_callback_f)(void*, int signal);

struct signal_manager
{
    struct list_head signal_handlers[MAX_SIGNAL_NUMBER];

    // the signal_enabled callback is called when some signal goes from no handlers to at least one handler
    signal_manager_callback_f signal_enabled;

    // the signal_enabled callback is called when some signal isn't useful anymore
    signal_manager_callback_f signal_disabled;

    void (*pre_fork_hook)(void *hook_state);
    void (*post_fork_hook)(void *hook_state, pid_t pid);
    void *hook_state;
};

struct signal_handler
{
    struct list_head __list;
    struct signal_manager *sigman;
    int signal;
    void (*handle)(struct errcont *errcont, struct signal_handler*);
    void (*remove)(struct signal_handler*);
};

extern void signal_manager_init(struct signal_manager *sigman);
extern void signal_manager_setup_handler(struct signal_manager *sigman, struct signal_handler *handler, int signal, bool head_handler);
extern void signal_manager_dispatch(struct signal_manager *sigman, struct errcont *errcont, int signal);
extern void signal_handler_del(struct signal_handler *handler);
extern pid_t managed_fork(struct signal_manager *sigman);
