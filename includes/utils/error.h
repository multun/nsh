#pragma once

#include "utils/lineinfo.h"
#include "utils/attr.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>

/**
** \brief represents a type of exception
** \details this is nothing more than a link time constant.
**   the reserved field is required for the structure to be valid
*/
struct ex_class
{
    // this structure isn't useful for anything but to use the
    // linker as a way to differentiate exceptions
    char reserved[1];
};

/**
** \brief a per-thread error context
** \details this structure is required in order to store some information
**   about the current error context
*/
struct ex_context
{
    const struct ex_class *class;
    int retcode;
};

/**
** \brief describes an exception scope
** \details this structure holds all the data required to raise an exception:
**   the parent scope can be used to go up the stack, and the exception context
**   can store information about the exception being thrown.
*/

struct ex_scope
{
    struct ex_scope *father;
    jmp_buf env;
    struct ex_context *context;
};

#define EXCEPTION_SCOPE(Man, Father)                                                     \
    (struct ex_scope)                                                                    \
    {                                                                                    \
        .context = (Man),                                                                \
        .father = (Father)                                                               \
    }

/**
** \fn void shraise(struct ex_scope *ex_scope, const struct ex_class *class)
** \brief raise an exception
** \details uses longjmp to go to the exception handler
** \param ex_scope the exception scope to raise into
** \param class the exception class to raise
*/
void ATTR(noreturn) shraise(struct ex_scope *ex_scope, const struct ex_class *class);

/**
** \brief prints line information, a message, and exit using shraise
** \param lineinfo the line-related metadata
** \param ex_scope the exception scope to raise the exception in
** \param ex_class the exception class being thrown
** \param format the error message's format string
*/
void ATTR(noreturn) sherror(const struct lineinfo *lineinfo, struct ex_scope *ex_scope,
                            const struct ex_class *ex_class, const char *format, ...);

void ATTR(noreturn) vsherror(const struct lineinfo *li, struct ex_scope *ex_scope,
                             const struct ex_class *ex_class, const char *format, va_list ap);
void vshwarn(const struct lineinfo *li, const char *format, va_list ap);
