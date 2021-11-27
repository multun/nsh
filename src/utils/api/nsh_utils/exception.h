#pragma once

#include <nsh_utils/lineinfo.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/error.h>

#include <setjmp.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

/**
** \brief represents a type of exception
** \details this is nothing more than a link time constant.
**   the reserved field is required for the structure to be valid
*/
struct exception_type
{
    // the error type this kind of exception maps to
    nsh_err_t compat_error;
};

/**
** \brief a per-thread error context
** \details this structure is required in order to store some information
**   about the current error context
*/
struct exception_context
{
    const struct exception_type *class;
};

/**
** \brief describes an exception scope
** \details this structure holds all the data required to raise an exception:
**   the parent scope can be used to go up the stack, and the exception context
**   can store information about the exception being thrown.
*/
struct exception_catcher
{
    struct exception_catcher *father;
    jmp_buf env;
    struct exception_context *context;
};

#define EXCEPTION_CATCHER(Man, Father)                                                   \
    (struct exception_catcher)                                                           \
    {                                                                                    \
        .context = (Man), .father = (Father)                                             \
    }

/**
** \fn void shraise(struct exception_catcher *catcher, const struct exception_type *class)
** \brief raise an exception
** \details uses longjmp to go to the exception handler
** \param catcher the exception scope to raise into
** \param class the exception class to raise
*/
void __noreturn shraise(struct exception_catcher *catcher,
                        const struct exception_type *class);

/**
** \fn void shraise(struct exception_catcher *catcher)
** \brief re-raises the last thrown exception for the context
*/
void __noreturn shreraise(struct exception_catcher *catcher);

/**
** \brief prints line information, a message, and exit using shraise
** \param lineinfo the line-related metadata
** \param catcher the exception scope to raise the exception in
** \param exception_type the exception class being thrown
** \param format the error message's format string
*/
void __noreturn sherror(const struct lineinfo *lineinfo,
                        struct exception_catcher *catcher,
                        const struct exception_type *exception_type, const char *format,
                        ...);

void __noreturn vsherror(const struct lineinfo *li, struct exception_catcher *catcher,
                         const struct exception_type *exception_type, const char *format,
                         va_list ap);


#define EXCEPTION_COMPAT_STUB(...)                                                       \
    do {                                                                                 \
        struct exception_context _context;                                               \
        struct exception_catcher _catcher;                                               \
        _catcher.context = &_context;                                                    \
        _catcher.father = NULL;                                                          \
        if (setjmp(_catcher.env)) {                                                      \
            return _context.class->compat_error;                                         \
        } else {                                                                         \
            struct exception_catcher *compat_catcher = &_catcher;                        \
            __VA_ARGS__;                                                                 \
            return NSH_OK;                                                               \
        }                                                                                \
    } while (0)


#define EXCEPTION_COMPAT_STUB_RETVAL(Res, ...)                                           \
    do {                                                                                 \
        struct exception_context _context;                                               \
        struct exception_catcher _catcher;                                               \
        _catcher.context = &_context;                                                    \
        _catcher.father = NULL;                                                          \
        if (setjmp(_catcher.env)) {                                                      \
            return _context.class->compat_error;                                         \
        } else {                                                                         \
            struct exception_catcher *compat_catcher = &_catcher;                        \
            *Res = __VA_ARGS__;                                                          \
            return NSH_OK;                                                               \
        }                                                                                \
    } while (0)
