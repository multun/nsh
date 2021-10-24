#pragma once

#include <nsh_utils/error.h>

/**
** \brief an exception class for loop breaking
** \details a handler is registered inside for and while loops
*/
extern struct ex_class g_ex_break;

/**
** \brief an exception class for loop continues
** \details a handler is registered inside for and while loops
*/
extern struct ex_class g_ex_continue;
