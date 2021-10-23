#pragma once

#include "io/cstream.h"


struct cstream_readline {
    struct cstream base;

    char *current_line;

    // position in the current line
    size_t line_position;
};

/**
** \brief initializes a readline stream
** \return a readline stream
*/
void cstream_readline_init(struct cstream_readline *cs);
