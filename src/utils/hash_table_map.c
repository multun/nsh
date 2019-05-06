#include "utils/hash_table.h"

void htable_map(s_htable *htable, void (*func)(struct pair *ptr))
{
    for (size_t i = 0; i < htable->capacity; i++) {
        struct pair *pp = NULL;
        struct pair *fp = htable->tab[i];
        while (fp || pp) {
            if (pp)
                func(pp);
            pp = fp;
            if (fp)
                fp = fp->next;
        }
    }
}
