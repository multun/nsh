#include <nsh_utils/evect.h>

#define GVECT_NAME evect
#define GVECT_TYPE char
#include <nsh_utils/gvect.defs>

void evect_push_string(struct evect *vect, const char *str)
{
    for (; *str; str++)
        evect_push(vect, *str);
}
