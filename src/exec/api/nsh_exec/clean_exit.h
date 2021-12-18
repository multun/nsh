#pragma once

#include <nsh_utils/attr.h>

/**

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
** \brief cleanly exits, provided that error codes are propagated
** \param env the environment which will store the error code
** \param retcode the status code to return with
*/
__unused_result int clean_exit(struct environment *env, int retcode);
