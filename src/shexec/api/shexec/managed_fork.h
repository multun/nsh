#pragma once

#include <unistd.h>
#include "shexec/environment.h"


extern pid_t managed_fork(struct environment *env);
