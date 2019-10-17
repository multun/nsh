#include <stdlib.h>

#include "ast/wordlist.h"
#include "shexec/args.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

static size_t wordlist_argc_count(struct wordlist *wl)
{
    size_t argc = 0;
    for (struct wordlist *tmp = wl; tmp; tmp = tmp->next)
        argc++;
    return argc;
}

static int wordlist_to_argv_sub(char **volatile *res, struct wordlist *volatile wl, struct environment *env,
                                struct errcont *cont)
{
    size_t argc = wordlist_argc_count(wl);
    char **argv = *res = calloc(sizeof(char *), (argc + 1));
    for (size_t i = 0; i < argc; (wl = wl->next), i++)
        argv[i] = expand(NULL, wl->str, env, cont);
    return argc;
}

// this function is here just in case res == &env->argv, so that expansion of
// the parameters is done within the proper context
int wordlist_to_argv(char ***res, struct wordlist *wl, struct environment *env, struct errcont *cont)
{
    char **volatile new_argv = NULL;
    struct keeper keeper = KEEPER(cont->keeper);
    if (setjmp(keeper.env)) {
        argv_free(new_argv);
        shraise(cont, NULL);
    } else {
        int argc = wordlist_to_argv_sub(&new_argv, wl, env, &ERRCONT(cont->errman, &keeper));
        *res = new_argv;
        return argc;
    }
}

void wordlist_free(struct wordlist *wl, bool free_buf)
{
    if (!wl)
        return;
    if (free_buf && wl->str)
        free(wl->str);
    wordlist_free(wl->next, free_buf);
    free(wl);
}
