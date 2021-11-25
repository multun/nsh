#pragma once

#include <nsh_io/cstream.h>
#include <nsh_exec/repl.h>


struct cstream_readline
{
    struct cstream base;

    // This is useful for getting the prompt,
    // as well as the aliasing context
    struct repl *repl;

    char *current_line;

    // position in the current line
    size_t line_position;
};

/**
** \brief initializes a readline stream
** \return a readline stream
*/
struct cstream_readline *cstream_readline_create(struct repl *repl);
