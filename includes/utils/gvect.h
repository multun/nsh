#include <stddef.h>
#include "utils/attr.h"

#ifndef GVECT_NAME
#error undefined GVECT_NAME in generic vector
#endif

#ifndef GVECT_TYPE
#error undefined GVECT_TYPE in generic vector
#endif

#define GVECT_U_CONCAT_(A, B) A ## _ ## B
#define GVECT_U_CONCAT(A, B) GVECT_U_CONCAT_(A, B)
#define GVECT_FNAME(Suffix) GVECT_U_CONCAT(GVECT_NAME, Suffix)

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

#define EVECT_INITIALIZED(Vect) (!!(Vect)->data)

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
