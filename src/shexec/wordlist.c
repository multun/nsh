#include "shparse/wordlist.h"
#include "shexec/args.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

#include <stdlib.h>

static void wordlist_expand_sub(struct cpvect *res, struct wordlist *wl,
                                struct environment *env,
                                struct errcont *cont)
{
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        struct shword *word = wordlist_get(wl, i);
        char *word_data = shword_buf(word);
        char *expanded = expand_nosplit(&word->line_info, word_data, env, cont);
        cpvect_push(res, expanded);
    }
}

// this function is here just in case res == &env->argv, so that expansion of
// the parameters is done within the proper context
void wordlist_expand(struct cpvect *res, struct wordlist *wl,
                     struct environment *env, struct errcont *cont)
{
    cpvect_init(res, wordlist_size(wl) + 1);

    /* on exception, free the result array */
    struct keeper keeper = KEEPER(cont->keeper);
    if (setjmp(keeper.env)) {
        /* free the result vector elements */
        for (size_t i = 0; i < cpvect_size(res); i++)
            free(cpvect_get(res, i));
        cpvect_destroy(res);
        shraise(cont, NULL);
    }

    /* expand the argument list */
    wordlist_expand_sub(res, wl, env, &ERRCONT(cont->errman, &keeper));
}
