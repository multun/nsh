#pragma once

#include <stdbool.h>
#include "shexec/environment.h"

/**
** \brief sets either PWD or OLDPWD to the current working directory
** \param var the variable to set
** \param env the environment to work with
*/
void update_pwd(const char *var, struct environment *env);
