#pragma once

#include "utils/signal_manager.h"


struct signal_manager_pipe
{
    struct signal_manager base;
};


extern void signal_manager_pipe_init(struct signal_manager_pipe *sigman_pipe);
