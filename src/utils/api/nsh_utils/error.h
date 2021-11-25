#pragma once

#include <stdio.h>

enum nsh_error
{
    // when a function returns NSH_OK, everything went well.
    // because NSH_OK == 0, testing if an error occured is easy
    NSH_OK = 0,

    // these errors signal something went wrong in
    // one of these modules
    NSH_IO_ERROR,
    NSH_LEXER_ERROR,
    NSH_PARSER_ERROR,
    NSH_EXECUTION_ERROR,

    // these aren't really errors, but behave in a similar way:
    // they interrupt normal program operation. The only difference
    // with other error kinds is that they are part of normal execution.
    NSH_KEYBOARD_INTERUPT,
    NSH_BREAK_INTERUPT,
    NSH_CONTINUE_INTERUPT,
    NSH_EXIT_INTERUPT,
};

typedef enum nsh_error nsh_err_t;

/** \brief Calls warn and return err */
nsh_err_t error_warn(nsh_err_t err, const char *fmt, ...);

/** \brief Calls warnx and return err */
nsh_err_t error_warnx(nsh_err_t err, const char *fmt, ...);
