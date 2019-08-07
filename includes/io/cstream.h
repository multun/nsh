#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils/attr.h"
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
struct io_backend
{
    f_io_reader reader;
    f_io_destructor dest;
};

/**
** \brief a cstream is a character stream, similar to FILE*
** \details it can also read from a readline or string input
*/
typedef struct cstream
{
    struct io_backend *backend;

    // whether the stream is interactive
    bool interactive;

    // the context the stream runs in. This is useful for getting the prompt,
    // as well as the aliasing context
    struct context *context;

    // the line-related informations, updated on each read
    s_lineinfo line_info;

    // whether buf contains some data
    bool has_buf;

    // a buffer enabling the user to peek data from the stream
    int buf;

    // an error context, which is only needed in a very specific case:
    // when a readline stream gets a keyboard interupt.
    // in all other cases, this can be NULL.
    struct errcont *errcont;
} s_cstream;


__unused static void cstream_set_errcont(struct cstream *cs, struct errcont *errcont)
{
    cs->errcont = errcont;
}

__unused static void cstream_check(struct cstream *cs) {
    assert(cs->backend != NULL);
    assert(cs->context != NULL);
    assert(cs->line_info.source != NULL);
}

int cstream_file_setup(FILE **file, const char *path, bool missing_ok);

/**
** \brief constructs a base stream, initialized with sane default values
** \details using this interface ensures destructing the stream does not exploit
**    undefined behavior
** \return a partialy uninitialized stream
*/
void cstream_init(struct cstream *cs, struct io_backend *backend, bool interactive);

/**
** \brief destructs a stream and all the underlying resources
** \param cs the stream to destroy
*/
void cstream_destroy(struct cstream *cs);

/**
** \brief peek a character from a stream
** \details returns without removing the next character of the stream
**   the second call in a row to peek always returns the same character
** \param cs the stream to peek from
** \return the next character on the stream
*/
int cstream_peek(struct cstream *cs);

/**
** \brief pop a character from a stream
** \details returns and removes a character from the stream
** \param cs the stream to peek from
** \return the next character on the stream
*/
int cstream_pop(struct cstream *cs);

/**
** \param cs the stream involved
** \return has the stream reached eof?
*/
bool cstream_eof(struct cstream *cs);


#include "io/cstream_readline.h"
#include "io/cstream_file.h"
#include "io/cstream_string.h"
