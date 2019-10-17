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
** \brief a kind of exception handler
** \details the current environment can be stored inside env, and
**   the parent context inside father
*/
struct keeper
{
#ifndef NDEBUG
    struct keeper *father;
#endif
    jmp_buf env;
};

/**
** \brief a per-thread error context
** \details this structure is required in order to store some information
**   about the current error context
*/
struct errman
{
    const struct ex_class *class;
    int retcode;
};

/**
** \brief detailsribes an error context
** \details this structure holds all the data required to raise an exception:
**   the keeper state can be uesd to go up the stack, and the error manager
**   can store information about the exception being thrown.
*/
struct errcont
{
    struct errman *errman;
    struct keeper *keeper;
};

#define ERRMAN                                                                           \
    (struct errman)                                                                           \
    {                                                                                    \
        .class = NULL,                                                                   \
    }

#define ERRCONT(Man, Keeper)                                                             \
    (struct errcont)                                                                          \
    {                                                                                    \
        .errman = (Man), .keeper = (Keeper)                                              \
    }

#ifndef NDEBUG
#    define KEEPER(Father)                                                               \
        (struct keeper)                                                                       \
        {                                                                                \
            .father = (Father)                                                           \
        }
#else
#    define KEEPER(Father) ((struct keeper){0})
#endif

/**
** \fn void shraise(struct errcont *cont, const struct ex_class *class)
** \brief raise an exception
** \details uses longjmp to notify the closest keeper
**   in order to avoid allocated data being lost, all allocations
**   between the keeper and the shraise call should be able to be
**   cleaner up by the keeper.
** \param cont the context to raise into
** \param class the exception class to raise
*/
void ATTR(noreturn) shraise(struct errcont *cont, const struct ex_class *class);

/**
** \brief prints line information, a message, and exit using shraise
** \param lineinfo the line-related metadata
** \param cont the error context to raise the exception in
** \param ex_class the exception class being thrown
** \param format the error message's format string
*/
void ATTR(noreturn) sherror(const struct lineinfo *lineinfo, struct errcont *cont,
                            const struct ex_class *ex_class, const char *format, ...);

void ATTR(noreturn) vsherror(const struct lineinfo *li, struct errcont *cont,
                             const struct ex_class *ex_class, const char *format, va_list ap);
void vshwarn(const struct lineinfo *li, const char *format, va_list ap);
