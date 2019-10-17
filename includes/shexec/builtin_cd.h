#pragma once

#include <stdbool.h>
#include "shexec/environment.h"

/**
** \brief sets either PWD or OLDPWD to the current working directory
** \param oldpwd whether oldpwd should be set instead of pwd
** \param env the environment to work with
*/
void update_pwd(bool oldpwd, struct environment*env);
