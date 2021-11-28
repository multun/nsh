#pragma once

#include <nsh_utils/attr.h>
#include <nsh_utils/exception.h>

/**
** \details clean_exit is thrown when the current objective is to
**   cleanly exit the program. Very little magic is done here, this
**   exception assumes a handler will take it into account.
*/
extern struct exception_type g_clean_exit;

/**
** \brief prints a formated error message and exits using clean_exit, just like err
** \param fmt a format string
*/
__unused_result int clean_err(struct environment *env, int retcode, const char *fmt, ...);

/**
** \brief prints a formated error message and exits using clean_exit, just like errx
** \param fmt a format string
*/
__unused_result int clean_errx(struct environment *env, int retcode, const char *fmt,
                               ...);

/**
** \brief cleanly exit by raising an exception
** \param cont the error context to raise into
** \param retcode the status code to return with
*/
__unused_result int clean_exit(struct environment *env, int retcode);
