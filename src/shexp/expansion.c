#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <string.h>

#include "shexec/clean_exit.h"
#include "shexec/builtin_shopt.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shexp/variable.h"
#include "utils/evect.h"
#include "shwlex/wlexer.h"
#include "shlex/lexer.h"

/* static bool predefined_lookup(char **res, s_env *env, char *var) */
/* { */
/*     for (size_t i = 0; var[i]; i++) */
/*         if (!isdigit(var[i])) */
/*             return false; */

/*     size_t iarg = atoi(var); */
/*     if (!iarg) */
/*         *res = env->progname; */
/*     else { */
/*         for (size_t i = 0; i < iarg; i++) */
/*             if (!env->argv[i]) { */
/*                 *res = NULL; */
/*                 return false; */
/*             } */
/*         *res = env->argv[iarg]; */
/*     } */

/*     free(var); */
/*     return true; */
/* } */

/* static bool special_var_lookup(char **res, s_env *env, char *var) */
/* { */
/*     bool found = false; */
/*     if (strlen(var) == 1) */
/*         found = special_char_lookup(res, env, *var); */
/*     else if ((found = !strcmp("RANDOM", var))) */
/*         expand_random(res); */
/*     else if ((found = !strcmp("UID", var))) */
/*         expand_uid(res); */
/*     else if ((found = !strcmp("SHELLOPTS", var))) */
/*         expand_shopt(res); */
/*     if (found) */
/*         free(var); */
/*     return found; */
/* } */

/* static char *var_lookup(s_env *env, char *var) */
/* { */
/*     char *look = NULL; */
/*     if (predefined_lookup(&look, env, var)) */
/*         return look; */
/*     if (special_var_lookup(&look, env, var)) */
/*         return look; */

/*     struct pair *var_pair = htable_access(env->vars, var); */
/*     free(var); */
/*     if (!var_pair) */
/*         return NULL; */
/*     s_var *nvar = var_pair->value; */
/*     return nvar->value; */
/* } */

/* static bool is_name_char(char c, bool first) */
/* { */
/*     return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' */
/*         || (!first && c >= '0' && c <= '9'); */
/* } */

/* static void fill_var(char **str, s_evect *vec, bool braces) */
/* { */
/*     evect_init(vec, strlen(*str)); */

/*     bool num = false; */
/*     if (is_name_char(**str, false)) */
/*         num = !is_name_char(**str, true); */
/*     else */
/*         return; */

/*     evect_push(vec, **str); */
/*     (*str)++; */

/*     for (; **str */
/*          && ((is_name_char(**str, false) && !num) */
/*              || (**str >= '0' && **str <= '9' && num)) */
/*          && (!braces || **str != '}'); */
/*          (*str)++) */
/*         evect_push(vec, **str); */

/*     if (braces) { */
/*         if (**str != '}') */
/*             warnx("%s: bad substitution. expected '}'", *str); */
/*         else */
/*             (*str)++; */
/*     } */
/* } */

/* static void expand_var(char **str, s_env *env, s_evect *vec) */
/* { */
/*     bool braces = **str == '{'; */
/*     if (braces) */
/*         (*str)++; */

/*     s_evect var; */
/*     fill_var(str, &var, braces); */

/*     size_t i = var.size; */
/*     if (!braces) */
/*         for (; i > 0; i--) */
/*             if (var_lookup(env, strndup(var.data, i))) */
/*                 break; */
/*     char *res = i ? var_lookup(env, strndup(var.data, i)) : NULL; */
/*     evect_destroy(&var); */

/*     if (res) */
/*         for (char *it = res; *it; it++) { */
/*             if (expansion_protected_char(*it)) */
/*                 evect_push(vec, '\\'); */
/*             evect_push(vec, *it); */
/*         } */
/* } */

/* static bool starts_expansion(char c) */
/* { */
/*     if (isalpha(c) || isdigit(c)) */
/*         return true; */

/*     switch (c) { */
/*     case '(': */
/*     case '{': */
/*     case '?': */
/*     case '@': */
/*     case '*': */
/*     case '$': */
/*     case '#': */
/*         return true; */
/*     default: */
/*         return false; */
/*     } */
/* } */

struct expansion_state {
    s_errcont *errcont;
    struct evect vec;
    s_env *env;
};

static void expand_guarded(struct expansion_state *exp_state,
                           struct wlexer *wlexer);

typedef enum wlexer_op (*f_expander)(struct expansion_state *exp_state,
                                     struct wlexer *wlexer, struct wtoken *wtoken);

static enum wlexer_op expand_regular(struct expansion_state *exp_state,
                                     struct wlexer *wlexer __unused,
                                     struct wtoken *wtoken)
{
    evect_push(&exp_state->vec, wtoken->ch[0]);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_eof(struct expansion_state *exp_state,
                                 struct wlexer *wlexer __unused,
                                 struct wtoken *wtoken __unused)
{
    if (wlexer->mode != MODE_UNQUOTED)
        clean_errx(exp_state->errcont, 1, "EOF while expecting quote during expansion");
    return LEXER_OP_RETURN;
}

static enum wlexer_op expand_squote(struct expansion_state *exp_state __unused,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    if (wlexer->mode == MODE_SINGLE_QUOTED)
        return LEXER_OP_RETURN;

    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED));
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_dquote(struct expansion_state *exp_state __unused,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    if (wlexer->mode == MODE_DOUBLE_QUOTED)
        return LEXER_OP_RETURN;

    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED));
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_btick(struct expansion_state *exp_state,
                                   struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    evect_push_string(&exp_state->vec, "btick_subshell<<");
    struct wlexer_btick_state btick_state = WLEXER_BTICK_INIT;
    WLEXER_BTICK_FOR(&btick_state, wtoken) {
        memset(wtoken, 0, sizeof(*wtoken));
        wlexer_pop(wtoken, wlexer);
        if (wtoken->type == WTOK_EOF)
            clean_errx(exp_state->errcont, 1, "unexpected EOF in ` section, during expansion");
        evect_push_string(&exp_state->vec, wtoken->ch);
    }
    evect_push_string(&exp_state->vec, ">>");
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_escape(struct expansion_state *exp_state,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    // clearing characters isn't safe if
    // the wlexer has some cached tokens
    assert(!wlexer_has_lookahead(wlexer));
    int ch = cstream_pop(wlexer->cs);
    if (ch == EOF)
        clean_errx(exp_state->errcont, 1, "unexpected EOF in escape, during expansion");

    evect_push(&exp_state->vec, ch);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_exp_subshell_open(struct expansion_state *exp_state,
                                               struct wlexer *wlexer,
                                               struct wtoken *wtoken __unused)
{
    struct wlexer sub_wlexer = WLEXER_FORK(wlexer, MODE_SUBSHELL);
    char *subshell_content = lexer_lex_string(exp_state->errcont, &sub_wlexer);
    expand_subshell(exp_state->errcont, subshell_content,  exp_state->env, &exp_state->vec);
    subshell_content = NULL; // the ownership was transfered to expand_subshell
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_invalid_state(struct expansion_state *exp_state,
                                              struct wlexer *wlexer __unused,
                                              struct wtoken *wtoken __unused)
{
    clean_errx(exp_state->errcont, 1, "reached an invalid state during expansion");
}

static f_expander expanders[] = {
    [WTOK_EOF] = expand_eof,
    [WTOK_REGULAR] = expand_regular,
    [WTOK_SQUOTE] = expand_squote,
    [WTOK_DQUOTE] = expand_dquote,
    [WTOK_BTICK] = expand_btick,
    [WTOK_ESCAPE] = expand_escape,
    [WTOK_EXP_SUBSH_OPEN] = expand_exp_subshell_open,
    [WTOK_EXP_SUBSH_CLOSE] = expand_invalid_state,
    [WTOK_SUBSH_OPEN] = expand_invalid_state,
    [WTOK_SUBSH_CLOSE] = expand_invalid_state,

    // TODO
    [WTOK_ARITH_OPEN] = expand_invalid_state,
    [WTOK_ARITH_CLOSE] = expand_invalid_state,
    [WTOK_EXP_OPEN] = expand_invalid_state,
    [WTOK_EXP_CLOSE] = expand_invalid_state,
};

static void expand_guarded(struct expansion_state *exp_state,
                           struct wlexer *wlexer)
{
    while (true) {
        struct wtoken wtoken;
        memset(&wtoken, 0, sizeof(wtoken));

        wlexer_pop(&wtoken, wlexer);
        enum wlexer_op op = expanders[wtoken.type](exp_state, wlexer, &wtoken);
        assert(op != LEXER_OP_FALLTHROUGH && op != LEXER_OP_PUSH);
        if (op & LEXER_OP_RETURN)
            return;
        if (op & LEXER_OP_CONTINUE)
            continue;
    }
}

char *expand(char *str, s_env *env, s_errcont *errcont)
{
    struct expansion_state exp_state = {
        .env = env,
    };

    evect_init(&exp_state.vec, strlen(str) + 1);
    struct keeper keeper = KEEPER(errcont->keeper);

    struct cstream cs = { 0 };
    cstream_string_init(&cs, str, "<in expansion>");
    struct wlexer wlexer;
    wlexer_init(&wlexer, &cs);

    if (setjmp(keeper.env)) {
        free(exp_state.vec.data);
        shraise(errcont, NULL);
    } else {
        s_errcont sub_errcont = ERRCONT(errcont->errman, &keeper);
        exp_state.errcont = &sub_errcont;
        expand_guarded(&exp_state, &wlexer);
    }

    evect_push(&exp_state.vec, '\0');
    return exp_state.vec.data;
}
