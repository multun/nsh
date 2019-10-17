#pragma once

#define GVECT_NAME evect
#define GVECT_TYPE char

#include "utils/gvect.h"

/**
** \brief push a string to the end of the vector
** \param vect the vector to push into
** \param str the string to push
*/
void evect_push_string(struct evect *vect, const char *str);

#undef GVECT_NAME
#undef GVECT_TYPE
