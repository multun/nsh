#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "cli/shopt.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/evect.h"
#include "utils/mprintf.h"
#include "utils/strutils.h"


static int expand_pid(struct expansion_state *exp_state)
{
    char res[UINT_MAX_CHARS(pid_t) + /* \0 */ 1];
    pid_t id = getpid();
    sprintf(res, "%u", id);
    expansion_push_string(exp_state, res);
    return 0;
}

static int expand_star_sep(struct expansion_state *exp_state, char sep)
{
    struct environment *env = exp_state->env;
    for (size_t i = 1; env->argv[i]; i++) {
        if (i > 1)
            expansion_push(exp_state, sep);
        expansion_push_string(exp_state, env->argv[i]);
    }
    return 0;
}

static int expand_star(struct expansion_state *exp_state)
{
    char sep = ' ';
    if (exp_state->IFS)
        sep = exp_state->IFS[0];
    return expand_star_sep(exp_state, sep);
}

static int expand_at(struct expansion_state *exp_state)
{
    // some contexts don't allow splitting (arith expansion, a=something)
    // sh -c 'IFS=l; a=$@; printf @%s@ "$a"' argv0 a b c
    // sh -c 'IFS=l; a=$*; printf @%s@ "$a"' argv0 a b c
    // sh -c 'IFS=garbage; echo $(($@))' argv0 1 + 2 '*' 3
    if (exp_state->quoting_mode == EXPANSION_QUOTING_NOSPLIT)
        return expand_star_sep(exp_state, ' ');

    struct environment *env = exp_state->env;
    for (size_t i = 1; env->argv[i]; i++) {
        if (i > 1)
            expansion_force_end_word(exp_state);
        expansion_push_string(exp_state, env->argv[i]);
    }
    return 0;
}

static int expand_sharp(struct expansion_state *exp_state)
{
    struct environment *env = exp_state->env;
    // the shell argc always is one step behind the C argc
    int argc = env->argc;

    // argc might be equal to 0 !
    // $ sh -c 'echo $#'
    // 0
    // $ sh -c 'echo $#' coucou
    // 0

    if (argc > 0)
        argc--;

    char res[INT_MAX_CHARS(int) + /* \0 */ 1];
    sprintf(res, "%d", argc);
    expansion_push_string(exp_state, res);
    return 0;
}

static int expand_return(struct expansion_state *exp_state)
{
    char res[UINT_MAX_CHARS(unsigned char) + /* \0 */ 1];
    unsigned char retcode = (256 + exp_state->env->code) % 256;
    sprintf(res, "%u", retcode);
    expansion_push_string(exp_state, res);
    return 0;
}

static int special_char_lookup(struct expansion_state *exp_state, char var)
{
    switch (var) {
    case '@':
        return expand_at(exp_state);
    case '*':
        return expand_star(exp_state);
    case '?':
        return expand_return(exp_state);
    case '$':
        return expand_pid(exp_state);
    case '#':
        return expand_sharp(exp_state);
    default:
        return 1;
    }
}

static int expand_random(struct expansion_state *exp_state)
{
    // $RANDOM isn't yet in POSIX
    // see https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html#tag_23_02_05_03
    // "This pseudo-random number generator was not seen as being useful to interactive users."
    char res[INT_MAX_CHARS(int) + /* \0 */ 1];
    sprintf(res, "%d", rand() % 32768);
    expansion_push_string(exp_state, res);
    return 0;
}

static int expand_uid(struct expansion_state *exp_state)
{
    char res[UINT_MAX_CHARS(uid_t) + /* \0 */ 1];
    uid_t id = getuid();
    sprintf(res, "%u", id);
    expansion_push_string(exp_state, res);
    return 0;
}

static int arguments_var_lookup(struct expansion_state *exp_state, char c)
{
    struct environment *env = exp_state->env;
    if (c < '0' || c > '9')
        return 1;

    size_t arg_index = c - '0';
    if (arg_index == 0) {
        expansion_push_string(exp_state, env->progname);
        return 0;
    }

    if ((int)arg_index >= env->argc)
        return 0;

    expansion_push_string(exp_state, env->argv[arg_index]);
    return 0;
}

static int special_var_lookup(struct expansion_state *exp_state, char *var)
{
    assert(var[0]);
    if (var[1] != '\0')
        return 1;


    if (arguments_var_lookup(exp_state, var[0]) == 0)
        return 0;

    return special_char_lookup(exp_state, *var);
}

static int expand_shopt(struct expansion_state *exp_state)
{
    bool first = true;
    for (size_t i = 0; i < SHOPT_COUNT; i++)
        if (g_shopts[i]) {
            if (!first)
                expansion_push(exp_state, ':');
            expansion_push_string(exp_state, string_from_shopt(i));
            first = false;
        }
    return 0;
}

static int builtin_var_lookup(struct expansion_state *exp_state, char *var)
{
    if (strcmp("RANDOM", var) == 0)
        return expand_random(exp_state);
    if (strcmp("UID", var) == 0)
        return expand_uid(exp_state);
    if (strcmp("SHELLOPTS", var) == 0)
        return expand_shopt(exp_state);
    return 1;
}

int expand_name(struct expansion_state *exp_state, char *var)
{
    if (special_var_lookup(exp_state, var) == 0)
        return 0;
    if (builtin_var_lookup(exp_state, var) == 0)
        return 0;

    const char *env_var;
    if ((env_var = environment_var_get(exp_state->env, var))) {
        expansion_push_string(exp_state, env_var);
        return 0;
    }
    return 1;
}
