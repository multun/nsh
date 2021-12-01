#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include <nsh_exec/environment.h>
#include <nsh_lex/variable.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/evect.h>
#include <nsh_utils/mprintf.h>
#include <nsh_utils/strutils.h>

#include "expansion.h"


static int expand_pid(struct expansion_state *exp_state)
{
    nsh_err_t err;
    char res[UINT_MAX_CHARS(pid_t) + /* \0 */ 1];
    pid_t id = getpid();
    sprintf(res, "%u", id);
    if ((err = expansion_push_splitable_string(exp_state, res)))
        return err;
    return EXPANSION_SUCCESS;
}

static int expand_star(struct expansion_state *exp_state)
{
    nsh_err_t err;
    char sep = exp_state->field_separator_joiner;
    struct environment *env = expansion_state_env(exp_state);
    for (size_t i = 1; env->argv[i]; i++) {
        if (i > 1)
            if ((err = expansion_push_splitable(exp_state, sep)))
                return err;
        if ((err = expansion_push_splitable_string(exp_state, env->argv[i])))
            return err;
    }
    return EXPANSION_SUCCESS;
}

static int expand_at(struct expansion_state *exp_state)
{
    nsh_err_t err;
    // some contexts don't allow splitting (arith expansion, a=something)
    // sh -c 'IFS=l; a=$@; printf @%s@ "$a"' argv0 a b c
    // sh -c 'IFS=l; a=$*; printf @%s@ "$a"' argv0 a b c
    // sh -c 'IFS=garbage; echo $(($@))' argv0 1 + 2 '*' 3
    if (exp_state->quoting_mode == EXPANSION_QUOTING_NOSPLIT)
        return expand_star(exp_state);

    struct environment *env = expansion_state_env(exp_state);
    for (size_t i = 1; env->argv[i]; i++) {
        if (i > 1)
            if ((err = expansion_end_word(exp_state)))
                return err;
        if ((err = expansion_push_splitable_string(exp_state, env->argv[i])))
            return err;
    }
    return EXPANSION_SUCCESS;
}

static int expand_sharp(struct expansion_state *exp_state)
{
    nsh_err_t err;
    struct environment *env = expansion_state_env(exp_state);
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
    if ((err = expansion_push_splitable_string(exp_state, res)))
        return err;
    return EXPANSION_SUCCESS;
}

static int expand_return(struct expansion_state *exp_state)
{
    nsh_err_t err;
    char res[UINT_MAX_CHARS(unsigned char) + /* \0 */ 1];
    unsigned char retcode = (256 + expansion_state_env(exp_state)->code) % 256;
    sprintf(res, "%u", retcode);
    if ((err = expansion_push_splitable_string(exp_state, res)))
        return err;
    return EXPANSION_SUCCESS;
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
        return EXPANSION_FAILURE;
    }
}

static int expand_random(struct expansion_state *exp_state)
{
    nsh_err_t err;

    // $RANDOM isn't yet in POSIX
    // see https://pubs.opengroup.org/onlinepubs/9699919799/xrat/V4_xcu_chap02.html#tag_23_02_05_03
    // "This pseudo-random number generator was not seen as being useful to interactive users."
    char res[INT_MAX_CHARS(int) + /* \0 */ 1];
    sprintf(res, "%d", rand() % 32768);
    if ((err = expansion_push_splitable_string(exp_state, res)))
        return err;
    return EXPANSION_SUCCESS;
}

static int expand_uid(struct expansion_state *exp_state)
{
    nsh_err_t err;

    char res[UINT_MAX_CHARS(uid_t) + /* \0 */ 1];
    uid_t id = getuid();
    sprintf(res, "%u", id);
    if ((err = expansion_push_splitable_string(exp_state, res)))
        return err;
    return EXPANSION_SUCCESS;
}

static int arguments_var_lookup(struct expansion_state *exp_state, char c)
{
    nsh_err_t err;
    struct environment *env = expansion_state_env(exp_state);
    if (c < '0' || c > '9')
        return EXPANSION_FAILURE;

    size_t arg_index = c - '0';
    if (arg_index == 0) {
        if ((err = expansion_push_splitable_string(exp_state, env->progname)))
            return err;
        return EXPANSION_SUCCESS;
    }

    if ((int)arg_index >= env->argc)
        return EXPANSION_SUCCESS;

    if ((err = expansion_push_splitable_string(exp_state, env->argv[arg_index])))
        return err;
    return EXPANSION_SUCCESS;
}

static int special_var_lookup(struct expansion_state *exp_state, const char *var)
{
    assert(var[0]);
    if (var[1] != '\0')
        return EXPANSION_FAILURE;

    if (arguments_var_lookup(exp_state, var[0]) == EXPANSION_SUCCESS)
        return EXPANSION_SUCCESS;

    return special_char_lookup(exp_state, *var);
}

static int expand_shopt(struct expansion_state *exp_state)
{
    nsh_err_t err;
    bool first = true;
    struct environment *env = expansion_state_env(exp_state);
    for (size_t i = 0; i < SHOPT_COUNT; i++) {
        if (!env->shopts[i])
            continue;

        if (!first) {
            if ((err = expansion_push_splitable(exp_state, ':')))
                return err;
            first = false;
        }

        if ((err = expansion_push_splitable_string(exp_state, string_from_shopt(i))))
            return err;
    }
    return EXPANSION_SUCCESS;
}

static int builtin_var_lookup(struct expansion_state *exp_state, const char *var)
{
    if (strcmp("RANDOM", var) == 0)
        return expand_random(exp_state);
    if (strcmp("UID", var) == 0)
        return expand_uid(exp_state);
    if (strcmp("SHELLOPTS", var) == 0)
        return expand_shopt(exp_state);
    return EXPANSION_FAILURE;
}

int expand_name(struct expansion_state *exp_state, const char *var_name)
{
    nsh_err_t err;

    if (special_var_lookup(exp_state, var_name) == EXPANSION_SUCCESS)
        return EXPANSION_SUCCESS;
    if (builtin_var_lookup(exp_state, var_name) == EXPANSION_SUCCESS)
        return EXPANSION_SUCCESS;

    struct sh_string *env_var;
    if ((env_var = environment_var_get_string(expansion_state_env(exp_state), var_name))
        == NULL)
        return EXPANSION_FAILURE;

    /* tell the expansion_state we're currently holding a reference to this variable.
       if this step is skipped, a reference will be lost when an exception occurs in
       expansion_push_splitable. */
    exp_state->scratch_value = &env_var->base;
    sh_string_get(env_var);
    if ((err = expansion_push_splitable_string(exp_state, sh_string_data(env_var))))
        return err;
    sh_string_put(env_var);
    exp_state->scratch_value = NULL;
    return EXPANSION_SUCCESS;
}
