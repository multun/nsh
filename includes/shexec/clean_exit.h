#pragma once

#include "utils/attr.h"
#include "utils/error.h"


/**
** \desc clean_exit is thrown when the current objective is to
**   cleanly exit the program. Very little magic is done here, this
**   exception assumes a handler will take it into account.
*/
extern s_ex_class g_clean_exit;


/**
** \brief prints a formated error message and exits using clean_exit
** \param fmt a format string
*/
void ATTR(noreturn) clean_err(s_errcont *cont, int retcode,
                              const char *fmt, ...);

/**
** \brief cleanly exit by raising an exception
** \param cont the error context to raise into
** \param retcode the status code to return with
*/
void ATTR(noreturn) clean_exit(s_errcont *cont, int retcode);
