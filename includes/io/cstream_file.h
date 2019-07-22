#pragma once

#include "io/cstream.h"


struct cstream_file {
    struct cstream base;

    bool close_on_exit;

    // a backend specific data pointer
    FILE *file;
};


/**
** \brief initializes a stream from a file
** \param source the source to display on error
** \param exit_close whether to close the stream on error
** \return a file stream
*/
void cstream_file_init(struct cstream_file *cs, FILE *stream, bool close_on_exit);
