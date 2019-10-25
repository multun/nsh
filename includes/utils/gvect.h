#include <stddef.h>
#include "utils/attr.h"
#include "gvect_common.h"

#ifndef GVECT_NAME
#error undefined GVECT_NAME in generic vector
#endif

#ifndef GVECT_TYPE
#error undefined GVECT_TYPE in generic vector
#endif

/**
** \brief a dynamicaly allocated vector
*/
struct GVECT_NAME
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
    GVECT_TYPE *data;
};

/**
** \brief initialize the vector to given size
** \param vect the vector to initialize
** \param capacity the future capacity of the vector
*/
void GVECT_FNAME(init)(struct GVECT_NAME *vect, size_t capacity);

/**
** \brief free the data stored in the vector
** \param vect the vector to destroy
*/
void GVECT_FNAME(destroy)(struct GVECT_NAME *vect);

/**
** \brief push an item at the end of the vector
** \param vect the vector to push into
** \param c the item to push in
*/
void GVECT_FNAME(push)(struct GVECT_NAME *vect, GVECT_TYPE c);

static inline size_t GVECT_FNAME(size)(struct GVECT_NAME *vect)
{
    return vect->size;
}

static inline GVECT_TYPE *GVECT_FNAME(data)(struct GVECT_NAME *vect)
{
    return vect->data;
}

static inline GVECT_TYPE GVECT_FNAME(get)(struct GVECT_NAME *vect, size_t i)
{
    return vect->data[i];
}

static inline GVECT_TYPE *GVECT_FNAME(last)(struct GVECT_NAME *vect)
{
    return &vect->data[vect->size - 1];
}
