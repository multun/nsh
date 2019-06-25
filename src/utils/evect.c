#include "utils/alloc.h"
#include "utils/evect.h"

#include <stdlib.h>

void evect_init(s_evect *vect, size_t capacity)
{
    vect->size = 0;
    vect->capacity = capacity;
    vect->data = xmalloc(capacity);
}

void evect_destroy(s_evect *vect)
{
    free(vect->data);
}

void evect_push(s_evect *vect, char c)
{
    if (vect->size == vect->capacity) {
        vect->capacity = 2 * vect->capacity + 1; // TODO: check for overflow
        vect->data = xrealloc(vect->data, vect->capacity);
    }
    vect->data[vect->size++] = c;
}

void evect_push_string(s_evect *vect, const char *str) {
    for (; *str; str++)
        evect_push(vect, *str);
}
