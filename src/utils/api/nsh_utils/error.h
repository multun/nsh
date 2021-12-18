#pragma once

#include <stdio.h>

/**
 * Error codes are negative to allow compatibility with positive integer
 * Return codes
 */
enum nsh_error
{
    // when a function returns NSH_OK, everything went well.
    // because NSH_OK == 0, testing if an error occured is easy
    NSH_OK = 0,

    // these errors signal something went wrong in
    // one of these modules
    NSH_IO_ERROR = -1,
    NSH_LEXER_ERROR = -2,
    NSH_PARSER_ERROR = -3,
    NSH_EXECUTION_ERROR = -4,
    NSH_EXPANSION_ERROR = -5,

    // these aren't really errors, but behave in a similar way:
    // they interrupt normal program operation. The only difference
    // with other error kinds is that they are part of normal execution.
    NSH_KEYBOARD_INTERUPT = -6,
    NSH_BREAK_INTERUPT = -7,
    NSH_CONTINUE_INTERUPT = -8,
    NSH_EXIT_INTERUPT = -9,
};

typedef enum nsh_error nsh_err_t;

/** \brief Calls warn and return err */
nsh_err_t error_warn(nsh_err_t err, const char *fmt, ...);

/** \brief Calls warnx and return err */
nsh_err_t error_warnx(nsh_err_t err, const char *fmt, ...);


/** \brief returns a short description of the error */
const char *nsh_error_repr(enum nsh_error err);
