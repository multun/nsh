#pragma once

#include <stddef.h>
#include <signal.h>

#include <nsh_utils/signal_list.h>


#define SIGNAL_READ_SIZE 1024


struct signal_pipe
{
    int consumer_pipe;
    int producer_pipe;
};


#define SIGNAL_PIPE_INIT                                                                 \
    {                                                                                    \
        .consumer_pipe = -1, .producer_pipe = -1                                         \
    }


extern int signal_pipe_write(int signum);
extern void signal_pipe_handler(int signum);
extern int signal_pipe_read(struct signal_list *events);
extern int signal_pipe_init(void);
extern int signal_pipe_fd(void);
extern void signal_pipe_reset(void);
