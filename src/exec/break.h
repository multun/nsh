#pragma once

#include <nsh_utils/exception.h>

/**
** \brief an exception class for loop breaking
** \details a handler is registered inside for and while loops
*/
extern struct exception_type g_ex_break;

/**
** \brief an exception class for loop continues
** \details a handler is registered inside for and while loops
*/
extern struct exception_type g_ex_continue;
