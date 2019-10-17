#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/evect.h"
#include "utils/mprintf.h"

static char *expand_pid(void)
{
    pid_t id = getpid();
    return mprintf("%u", id);
}

static char *expand_args(struct environment *env)
{
    struct evect vec;
    evect_init(&vec, 10);
    if (*env->argv && env->argv[1])
        for (char *tmp = env->argv[1]; *tmp; tmp++)
            evect_push(&vec, *tmp);
    for (size_t i = 2; env->argv[i]; i++) {
        evect_push(&vec, ' ');
        for (char *tmp = env->argv[i]; *tmp; tmp++)
            evect_push(&vec, *tmp);
    }
    evect_push(&vec, '\0');
    return vec.data;
}

static char *expand_sharp(struct environment *env)
{
    // the shell argc always is one step behind the C argc
    int argc = env->argc;

    // argc might be equal to 0 !
    // $ sh -c 'echo $#'
    // 0
    // $ sh -c 'echo $#' coucou
    // 0

    if (argc > 0)
        argc--;
    return mprintf("%d", argc);
}

static char *expand_return(struct environment *env)
{
    return mprintf("%u", (256 + env->code) % 256);
}

char *special_char_lookup(struct environment *env, char var)
{
    switch (var) {
    case '@':
        return expand_args(env);
    case '*':
        return expand_args(env);
    case '?':
        return expand_return(env);
    case '$':
        return expand_pid();
    case '#':
        return expand_sharp(env);
    default:
        return NULL;
    }
}

char *expand_random(void)
{
    // $RANDOM isn't yet in POSIX
    // see https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html#tag_23_02_05_03
    // "This pseudo-random number generator was not seen as being useful to interactive users."
    return mprintf("%d", rand() % 32768);
}

char *expand_uid(void)
{
    uid_t id = getuid();
    return mprintf("%u", id);
}
