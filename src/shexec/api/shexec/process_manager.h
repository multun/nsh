#pragma once

#include "shparse/ast.h"
#include "utils/pvect.h"
#include "utils/signal_manager.h"

struct process
{
    int pid;
    enum process_status
    {
        PROCESS_STATUS_RUNNING = 0,
        PROCESS_STATUS_DONE,
    } status;
};

struct job
{
    /* the ast to pretty-print the command from */
    struct shast *ast;

    struct termios ttystate;/* saved tty state for stopped jobs */
	pid_t	saved_ttypgrp;	/* saved tty process group for stopped jobs */

    /* the number of children in the processes array */
    size_t pipeline_size;
    struct process processes[];
};


#define GVECT_NAME proc_pipeline_vect
#define GVECT_TYPE struct process *
#include "utils/pvect_wrap.h"
#undef GVECT_NAME
#undef GVECT_TYPE


struct process_manager
{
    bool enabled;
    struct signal_handler sigchild_handler;
    struct proc_pipeline_vect pipelines;

/* rt_sigaction(SIGTSTP, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0 */
/* rt_sigaction(SIGTSTP, {sa_handler=SIG_IGN, sa_mask=~[RTMIN RT_1], sa_flags=SA_RESTORER, sa_restorer=0x7f6e2f05f470}, NULL, 8) = 0 */
/* rt_sigaction(SIGTTOU, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0 */
/* rt_sigaction(SIGTTOU, {sa_handler=SIG_IGN, sa_mask=~[RTMIN RT_1], sa_flags=SA_RESTORER, sa_restorer=0x7f6e2f05f470}, NULL, 8) = 0 */
/* rt_sigaction(SIGTTIN, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0 */
/* rt_sigaction(SIGTTIN, {sa_handler=SIG_DFL, sa_mask=~[RTMIN RT_1], sa_flags=SA_RESTORER, sa_restorer=0x7f6e2f05f470}, NULL, 8) = 0 */
};


extern void process_manager_init(struct process_manager *procman);
