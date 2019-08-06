#pragma once

#include <stddef.h>
#include "utils/attr.h"

/**
** \brief a dynamicaly allocated bytes vector
*/
typedef struct evect
{
    /**
    ** the current size of the vector
    */
    size_t size;
    /**
    ** the maximum capacity
    */
    size_t capacity;
    /**
    ** the data stored in the vector
    */
    char *data;
} s_evect;

#define EVECT_INITIALIZED(Vect) (!!(Vect)->data)

__unused static size_t evect_size(struct evect *vect)
{
    return vect->size;
}

__unused static char *evect_data(struct evect *vect)
{
    return vect->data;
}

/**
** \brief initialize the vector to given size
** \param vect the vector to initialize
** \param capacity the future capacity of the vector
*/
void evect_init(s_evect *vect, size_t capacity);
/**
** \brief free the data stored in the vector
** \param vect the vector to destroy
*/
void evect_destroy(s_evect *vect);

/**
** \brief push a character to the end of the vector
** \param vect the vector to push into
** \param c the character to push in
*/
void evect_push(s_evect *vect, char c);

/**
** \brief push a string to the end of the vector
** \param vect the vector to push into
** \param str the string to push
*/
void evect_push_string(s_evect *vect, const char *str);
