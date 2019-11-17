#include "shparse/wordlist.h"
#include "shexec/args.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

#include <stdlib.h>

static int wordlist_to_argv_sub(char **volatile *res, struct wordlist *volatile wl, struct environment *env,
                                struct errcont *cont)
{
    size_t argc = wordlist_size(wl);

    char **argv = *res = xcalloc(sizeof(char *), argc + /* NULL */ 1);
    for (size_t i = 0; i < argc; i++)
    {
        struct shword *word = wordlist_get(wl, i);
        argv[i] = expand_nosplit(&word->line_info, shword_buf(word), env, cont);
    }
    return argc;
}

// this function is here just in case res == &env->argv, so that expansion of
// the parameters is done within the proper context
int wordlist_to_argv(char ***res, struct wordlist *wl, struct environment *env, struct errcont *cont)
{
    char **volatile new_argv = NULL;

    /* on exception, free the pending arguments array */
    struct keeper keeper = KEEPER(cont->keeper);
    if (setjmp(keeper.env)) {
        argv_free(new_argv);
        shraise(cont, NULL);
    }

    /* expand the argument list */
    int argc = wordlist_to_argv_sub(&new_argv, wl, env, &ERRCONT(cont->errman, &keeper));
    *res = new_argv;
    return argc;
}
