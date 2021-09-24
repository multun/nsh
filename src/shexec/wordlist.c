#include "shparse/wordlist.h"
#include "shexec/args.h"
#include "shexec/environment.h"
#include "shexec/expansion.h"
#include "utils/alloc.h"

#include <stdlib.h>

// this function is here to free the expanded array in case the expansion fails
void wordlist_expand(struct cpvect *res, struct wordlist *wl,
                     struct environment *env, struct ex_scope *ex_scope)
{
    cpvect_init(res, wordlist_size(wl) + 1);

    /* on exception, free the result array */
    struct ex_scope sub_ex_scope = EXCEPTION_SCOPE(ex_scope->context, ex_scope);
    if (setjmp(sub_ex_scope.env)) {
        /* free the result vector elements */
        for (size_t i = 0; i < cpvect_size(res); i++)
            free(cpvect_get(res, i));
        cpvect_destroy(res);
        shraise(ex_scope, NULL);
    }

    /* expand the argument list */
    expand_wordlist(res, wl, 0, env, &sub_ex_scope);
}
