#pragma once

#include <stdlib.h>

// all the includes below bring in exception types

#include <nsh_utils/error.h>
#include <nsh_utils/exception.h>

#include <nsh_io/keyboard_interrupt.h>
#include <nsh_lex/lexer_error.h>
#include <nsh_parse/parser_error.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_exec/clean_exit.h>

#include "break.h"

/**
 * \page Migrating from exceptions to errors
 *
 * When this project started, it was deemed relevant to use exceptions.
 * This was mostly ill-advised, as it makes a lot of the code of the
 * project much harder than necessary to maintain, as we suddently have
 * to worry about exception safety, in addition to the huge amount of
 * boilerplate.
 *
 * This wrapper code sets up an exception context, and
 */


static inline void __noreturn raise_from_error(struct exception_catcher *catcher,
                                               nsh_err_t err)
{
    const struct exception_type *ex_class;
    if (err == NSH_KEYBOARD_INTERUPT)
        ex_class = &g_keyboard_interrupt;
    else if (err == NSH_LEXER_ERROR)
        ex_class = &g_lexer_error;
    else if (err == NSH_PARSER_ERROR)
        ex_class = &g_parser_error;
    else if (err == NSH_EXECUTION_ERROR)
        ex_class = &g_runtime_error;
    else if (err == NSH_EXIT_INTERUPT)
        ex_class = &g_clean_exit;
    else if (err == NSH_BREAK_INTERUPT)
        ex_class = &g_ex_break;
    else if (err == NSH_CONTINUE_INTERUPT)
        ex_class = &g_ex_continue;
    else
        abort();
    shraise(catcher, ex_class);
}
