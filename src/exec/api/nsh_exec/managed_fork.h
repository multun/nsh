#pragma once

#include <unistd.h>
#include <nsh_exec/environment.h>


extern pid_t managed_fork(struct environment *env);
