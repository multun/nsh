#include "shparse/wordlist.h"
#include "shexec/args.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

#include <stdlib.h>

// this function is here to free the expanded array in case the expansion fails
void wordlist_expand(struct cpvect *res, struct wordlist *wl,
                     struct environment *env, struct errcont *errcont)
{
    cpvect_init(res, wordlist_size(wl) + 1);

    /* on exception, free the result array */
    struct errcont sub_errcont = ERRCONT(errcont->errman, errcont);
    if (setjmp(sub_errcont.env)) {
        /* free the result vector elements */
        for (size_t i = 0; i < cpvect_size(res); i++)
            free(cpvect_get(res, i));
        cpvect_destroy(res);
        shraise(errcont, NULL);
    }

    /* expand the argument list */
    expand_wordlist(res, wl, 0, env, &sub_errcont);
}
