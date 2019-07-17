#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "utils/evect.h"
#include "utils/lineinfo.h"

struct cstream;
struct context;

/**
** \brief A pointer to a function able to read from a specific stream type
** \param a stream to pull data from
** \return a character, or EOF
*/
typedef int (*f_io_reader)(struct cstream *cs);

/**
** \brief A pointer to a function destructing a specific stream type instance
** \param a stream to destruct
*/
typedef void (*f_io_destructor)(struct cstream *cs);

/**
** \brief io backends are structures holding stream type specific routines
*/
typedef struct io_backend
{
    f_io_reader reader;
    f_io_destructor dest;
} s_io_backend;

/**
** \brief a cstream is a character stream, similar to FILE*
** \details it can also read from a readline or string input
*/
typedef struct cstream
{
    // whether the stream is interactive
    bool interactive;

    // the context the stream runs in. This is useful for getting the prompt,
    // as well as the aliasing context
    struct context *context;

    s_io_backend *backend;
    // the line-related informations, updated on each read
    s_lineinfo line_info;

    // whether buf contains some data
    bool has_buf;

    // a buffer enabling the user to peek data from the stream
    int buf;

    // whether the stream reached EOF
    bool eof;

    /*
    ** Backend-specific data
    ** TODO: move to separate structs
    */

    // contains each poped character since the beginning of the line.
    // it might seem useless or awkward to have this here, but the current
    // readline backend needs it
    s_evect linebuf;

    // should the stream be closed on exit ? only meaningful for the file backend
    bool exit_close;

    // a backend-dependant cursor
    size_t back_pos;

    // a backend specific data pointer
    void *data;
} s_cstream;

/**
** \brief constructs a base stream, initialized with sane default values
** \details using this interface ensures destructing the stream does not exploit
**    undefined behavior
** \return a partialy uninitialized stream
*/
s_cstream *cstream_create_base(void);

/**
** \brief initializes a stream from a file
** \param source the source to display on error
** \param exit_close whether to close the stream on error
** \return a file stream
*/
s_cstream *cstream_from_file(FILE *stream, const char *source, bool exit_close);

/**
** \brief initializes a readline stream
** \return a readline stream
*/
s_cstream *cstream_readline(void);

/**
** \brief initializes a stream from a string
** \param source the source to display on error
** \return a string stream
*/
s_cstream *cstream_from_string(char *string, const char *source, struct lineinfo *parent);

void cstream_string_init(struct cstream *cstream, char *string, const char *source, struct lineinfo *parent);

/**
** \brief destructs a stream and all the underlying resources
** \param cs the stream to destroy
*/
void cstream_free(s_cstream *cs);

/**
** \brief peek a character from a stream
** \details returns without removing the next character of the stream
**   the second call in a row to peek always returns the same character
** \param cs the stream to peek from
** \return the next character on the stream
*/
int cstream_peek(s_cstream *cs);

/**
** \brief pop a character from a stream
** \details returns and removes a character from the stream
** \param cs the stream to peek from
** \return the next character on the stream
*/
int cstream_pop(s_cstream *cs);

/**
** \param cs the stream involved
** \return has the stream reached eof?
*/
bool cstream_eof(s_cstream *cs);

extern s_io_backend g_io_file_backend;
extern s_io_backend g_io_readline_backend;
extern s_io_backend g_io_string_backend;
