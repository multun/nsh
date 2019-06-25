#include <string.h>
#include <stdio.h>
#include <sys/random.h>
#include <unistd.h>
#include <err.h>

#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/alloc.h"
#include "utils/evect.h"
#include "utils/mprintf.h"

static char *expand_pid(void)
{
    pid_t id = getpid();
    return mprintf("%u", id);
}

static char *expand_args(s_env *env)
{
    s_evect vec;
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

static char *expand_sharp(s_env *env)
{
    size_t argc = 0;
    while (env->argv[argc])
        argc++;

    if (argc)
        argc--;
    return mprintf("%zu", argc);
}

static char *expand_return(s_env *env)
{
    return mprintf("%u", (256 + env->code) % 256);
}

char *special_char_lookup(s_env *env, char var)
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
    int rnd = 0;
    getrandom(&rnd, sizeof(int), 0);
    return mprintf("%d", (rnd % 32768 + 32768) % 32768);
}

char *expand_uid(void)
{
    uid_t id = getuid();
    return mprintf("%u", id);
}
