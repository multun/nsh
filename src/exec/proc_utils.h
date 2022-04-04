#pragma once

#include <sys/wait.h>


/**
 * \brief an easy to use alternative to waitpid
 * \return a negative value in case of error, or exit status,
 *         or 128+n if the command is terminated by signal n.
 */
int proc_wait_exit(pid_t pid);
