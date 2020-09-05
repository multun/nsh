#include "shexec/process_manager.h"


static void sigchild_handler(struct ex_scope *ex_scope __unused, struct signal_manager *sigman __unused, struct signal_handler *handler __unused, int signal __unused)
{

}


void process_manager_init(struct process_manager *procman)
{
    process_vect_init(&procman->processes, 4);
    procman->sigchild_handler.handle = sigchild_handler;
}

