#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/evect.h>
#include <nsh_utils/lineinfo.h>
#include <nsh_utils/error.h>


/** \brief EOF can't be used as it's negative */
#define CSTREAM_EOF 256


struct cstream;

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
typedef nsh_err_t (*f_io_destructor)(struct cstream *cs);

/**
** \brief A pointer to a function resetting a stream
** \param a stream to reset
*/
typedef nsh_err_t (*f_io_reset)(struct cstream *cs);

/**
** \brief io backends are structures holding stream type specific routines
*/
struct io_backend
{
    f_io_reader reader;
    f_io_destructor dest;
    f_io_reset reset;
};

/**
** \brief a cstream is a character stream, similar to FILE*
** \details it can also read from readline or a string
*/
struct cstream
{
    struct io_backend *backend;

    // whether the stream is interactive
    bool interactive;

    // the line-related informations, updated on each read
    struct lineinfo line_info;

    // the number of read characters
    size_t offset;

    // whether buf contains some data
    bool has_buf;

    // a buffer enabling the user to peek data from the stream
    int buf;
};


nsh_err_t cstream_file_setup(FILE **file, const char *path, bool missing_ok);

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
nsh_err_t cstream_destroy(struct cstream *cs);

/**
** \brief destructs and deallocates a stream
** \param cs the struct to destroy and free
*/
nsh_err_t cstream_free(struct cstream *cs);

/**
** \brief peek a character from a stream
** \details returns without removing the next character of the stream
**   the second call in a row to peek always returns the same character
** \param cs the stream to peek from
** \return the next character on the stream
*/
int cstream_peek(struct cstream *cs) __unused_result;

/**
** \brief pop a character from a stream
** \details returns and removes a character from the stream
** \param cs the stream to peek from
** \return the next character on the stream
*/
int cstream_pop(struct cstream *cs) __unused_result;

/**
 * \brief Discards the next character in the stream.
 *        Can only be called after a successful peek.
 */
void cstream_discard(struct cstream *cs);

/**
** \param cs the stream involved
** \return either a negative an error code, or whether the stream reached eof
*/
int cstream_eof(struct cstream *cs);

/** \brief prepare the stream to read new input, if relevant */
nsh_err_t cstream_reset(struct cstream *cs);


#include <nsh_io/cstream_file.h>
#include <nsh_io/cstream_string.h>
