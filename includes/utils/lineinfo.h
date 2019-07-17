#pragma once

#include <stddef.h>
#include <stdio.h>

/**
** \brief information about the current context
** \desc used for error messages
*/
typedef struct lineinfo
{
    /**
    ** the current line number
    */
    size_t line;
    /**
    ** the current column number
    */
    size_t column;
    /**
    ** a path to the source of the error
    */
    const char *source;
    /**
    ** A parent line info context
    */
    struct lineinfo *parent;
} s_lineinfo;

void lineinfo_print(const struct lineinfo *li, FILE *stream);

#define LINEINFO(Source, Parent)                                                         \
    (s_lineinfo)                                                                         \
    {                                                                                    \
        .line = 1, .column = 1, .source = (Source), .parent = (Parent)                   \
    }
