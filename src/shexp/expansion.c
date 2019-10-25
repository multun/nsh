#include "shexec/builtin_shopt.h"
#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/evect.h"
#include "utils/mprintf.h"
#include "shwlex/wlexer.h"
#include "shlex/lexer.h"
#include "shlex/variable.h"
#include "shexp/arithmetic_expansion.h"

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

static void expand_guarded(struct expansion_state *exp_state,
                           struct wlexer *wlexer);

void __noreturn expansion_error(struct expansion_state *exp_state, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsherror(exp_state->line_info, exp_state->errcont, &g_lexer_error, fmt, ap);

    va_end(ap);
}

void expansion_warning(struct expansion_state *exp_state, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vshwarn(exp_state->line_info, fmt, ap);

    va_end(ap);
}

static char *arguments_var_lookup(struct environment *env, char c)
{
    if (c < '0' || c > '9')
        return NULL;

    size_t arg_index = c - '0';
    if (arg_index == 0)
        return strdup(env->progname);

    if ((int)arg_index >= env->argc)
        return strdup("");
    return strdup(env->argv[arg_index]);
}

static char *special_var_lookup(struct environment *env, char *var)
{
    assert(var[0]);
    if (var[1] != '\0')
        return NULL;

    char *res;

    if ((res = arguments_var_lookup(env, var[0])))
        return res;

    return special_char_lookup(env, *var);
}

static char *builtin_var_lookup(char *var)
{
    if (!strcmp("RANDOM", var))
        return expand_random();
    else if (!strcmp("UID", var))
        return expand_uid();
    else if (!strcmp("SHELLOPTS", var))
        return expand_shopt();
    return NULL;
}

char *expand_name(struct environment *env, char *var)
{
    char *res;
    if ((res = special_var_lookup(env, var)))
        return res;
    if ((res = builtin_var_lookup(var)))
        return res;

    const char *env_var;
    if ((env_var = environment_var_get(env, var)))
        return strdup(env_var);
    return NULL;
}

static void expand_guarded(struct expansion_state *exp_state,
                           struct wlexer *wlexer);

typedef enum wlexer_op (*f_expander)(struct expansion_state *exp_state,
                                     struct wlexer *wlexer, struct wtoken *wtoken);

static enum wlexer_op expand_dollar(struct expansion_state *exp_state,
                                    struct wlexer *wlexer,
                                    struct wtoken *wtoken)
{
    struct wlexer exp_wlexer = WLEXER_FORK(wlexer, MODE_EXPANSION);
    struct variable_name var_name;
    variable_name_init(&var_name, 16); // reasonable variable name size

    do {
        memset(wtoken, 0, sizeof(*wtoken));
        wlexer_pop(wtoken, &exp_wlexer);
        if (wtoken->type == WTOK_EXP_CLOSE)
            break;

        assert(wtoken->type == WTOK_REGULAR);

        if (!variable_name_check(&var_name, wtoken->ch[0])) {
            variable_name_destroy(&var_name);
            expansion_error(exp_state, "invalid characted in ${} section: %c", wtoken->ch[0]);
        }

        variable_name_push(&var_name, wtoken->ch[0]);
    } while (true);

    variable_name_finalize(&var_name);
    char *var_content = expand_name(exp_state->env, var_name.simple_var.data);
    if (var_content != NULL)
        evect_push_string(&exp_state->vec, var_content);
    free(var_content);
    variable_name_destroy(&var_name);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_regular(struct expansion_state *exp_state,
                                     struct wlexer *wlexer,
                                     struct wtoken *wtoken)
{
    if (wlexer->mode == MODE_SINGLE_QUOTED || wtoken->ch[0] != '$') {
        evect_push(&exp_state->vec, wtoken->ch[0]);
        return LEXER_OP_CONTINUE;
    }

    // fetch the longest valid variable name
    struct variable_name var_name;
    variable_name_init(&var_name, 16); // reasonable variable name size
    do {
        struct wtoken next_tok;
        wlexer_peek(&next_tok, wlexer);
        if (next_tok.type != WTOK_REGULAR)
            break;
        if (!variable_name_check(&var_name, next_tok.ch[0]))
            break;
        variable_name_push(&var_name, next_tok.ch[0]);
        wlexer_clear_lookahead(wlexer);
    } while (true);

    // push lonely dollars as is
    if (var_name.simple_var.size == 0) {
        variable_name_destroy(&var_name);
        evect_push(&exp_state->vec, wtoken->ch[0]);
        return LEXER_OP_CONTINUE;
    }

    // add the trailing null byte
    variable_name_finalize(&var_name);

    // look for the variable value
    char *var_content = expand_name(exp_state->env, var_name.simple_var.data);
    if (var_content != NULL)
        evect_push_string(&exp_state->vec, var_content);
    free(var_content);

    variable_name_destroy(&var_name);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_eof(struct expansion_state *exp_state,
                                 struct wlexer *wlexer __unused,
                                 struct wtoken *wtoken __unused)
{
    if (wlexer->mode != MODE_UNQUOTED)
        // TODO: display the current mode
        expansion_error(exp_state, "EOF while expecting quote during expansion");
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
    struct evect btick_content;
    evect_init(&btick_content, 32); // hopefuly sane default :(
    struct wlexer_btick_state btick_state = WLEXER_BTICK_INIT;
    WLEXER_BTICK_FOR(&btick_state, wtoken) {
        memset(wtoken, 0, sizeof(*wtoken));
        wlexer_pop(wtoken, wlexer);
        // discard the final backtick
        if (wtoken->type == WTOK_EOF)
            expansion_error(exp_state, "unexpected EOF in ` section, during expansion");
        if (wlexer_btick_escaped(&btick_state))
            continue;
        if (wtoken->type == WTOK_BTICK)
            break;
        evect_push_string(&btick_content, wtoken->ch);
    }

    evect_push(&btick_content, '\0');
    expand_subshell(exp_state->errcont, btick_content.data, exp_state->env, &exp_state->vec);
    btick_content.data = NULL; // the ownership was transfered to expand_subshell
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
        expansion_error(exp_state, "unexpected EOF in escape, during expansion");

    evect_push(&exp_state->vec, ch);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_exp_subshell_open(struct expansion_state *exp_state,
                                               struct wlexer *wlexer,
                                               struct wtoken *wtoken __unused)
{
    struct wlexer sub_wlexer = WLEXER_FORK(wlexer, MODE_SUBSHELL);
    char *subshell_content = lexer_lex_string(exp_state->errcont, &sub_wlexer);
    expand_subshell(exp_state->errcont, subshell_content, exp_state->env, &exp_state->vec);
    subshell_content = NULL; // the ownership was transfered to expand_subshell
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_arith_open(struct expansion_state *exp_state,
                                        struct wlexer *wlexer,
                                        struct wtoken *wtoken __unused)
{
    int rc;

    // expand the content of the arithmetic expansion
    int initial_size = evect_size(&exp_state->vec);
    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_ARITH));

    // find the arithmetic expression
    evect_push(&exp_state->vec, '\0');
    char *arith_content = evect_data(&exp_state->vec) + initial_size;

    // prepare the arithmetic wlexer and stream
    struct cstream_string cs;
    cstream_string_init(&cs, arith_content);
    cs.base.line_info = LINEINFO("<expansion>", exp_state->line_info);

    struct arith_lexer alexer = {
        .exp_state = exp_state,
        .cs = &cs.base,
    };

    struct arith_value res_val;
    if ((rc = arith_parse(&res_val, &alexer, 0)))
        // FIXME: exception
        errx(1, "error in arith_open parsing");

    struct arith_token next_tok;
    if ((rc = arith_lexer_peek(&next_tok, &alexer)))
        // FIXME: exception
        errx(1, "error in arith_open peek");

    if (next_tok.type != &arith_type_eof)
        expansion_error(exp_state, "unexpected token: %s", next_tok.type->name);

    int res = arith_value_to_int(exp_state, &res_val);
    int res_size = snprintf(NULL, 0, "%d", res);
    char print_buf[res_size + /* \0 */ 1];
    sprintf(print_buf, "%d", res);
    exp_state->vec.size = initial_size;
    for (int i = 0; i < res_size; i++)
        evect_push(&exp_state->vec, print_buf[i]);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_arith_close(struct expansion_state *exp_state __unused,
                                         struct wlexer *wlexer,
                                         struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH);
    return LEXER_OP_RETURN;
}

static enum wlexer_op expand_invalid_state(struct expansion_state *exp_state,
                                           struct wlexer *wlexer __unused,
                                           struct wtoken *wtoken __unused)
{
    expansion_error(exp_state, "reached an invalid state during expansion");
}

static enum wlexer_op expand_arith_group_open(struct expansion_state *exp_state,
                                              struct wlexer *wlexer,
                                              struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH_GROUP || wlexer->mode == MODE_ARITH);
    // expand to the litteral value, so that the arithmetic interpreter can
    // re-parse it after parameter substitution
    evect_push(&exp_state->vec, '(');
    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_ARITH_GROUP));
    evect_push(&exp_state->vec, ')');
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_arith_group_close(struct expansion_state *exp_state __unused,
                                              struct wlexer *wlexer,
                                              struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH_GROUP);
    return LEXER_OP_RETURN;
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
    [WTOK_ARITH_OPEN] = expand_arith_open,
    [WTOK_ARITH_CLOSE] = expand_arith_close,
    [WTOK_EXP_OPEN] = expand_dollar,
    [WTOK_EXP_CLOSE] = expand_invalid_state,
    // these tokens should be pulled in from the arithmetic lexer
    [WTOK_ARITH_GROUP_OPEN] = expand_arith_group_open,
    [WTOK_ARITH_GROUP_CLOSE] = expand_arith_group_close,
};

static void expand_guarded(struct expansion_state *exp_state,
                           struct wlexer *wlexer)
{
    while (true) {
        struct wtoken wtoken;
        wlexer_pop(&wtoken, wlexer);
        enum wlexer_op op = expanders[wtoken.type](exp_state, wlexer, &wtoken);
        assert(op != LEXER_OP_FALLTHROUGH && op != LEXER_OP_PUSH);
        if (op & LEXER_OP_RETURN)
            return;
        if (op & LEXER_OP_CONTINUE)
            continue;
    }
}

char *expand(struct lineinfo *line_info, char *str, struct environment *env, struct errcont *errcont)
{
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<expansion>", line_info);

    struct expansion_state exp_state = {
        .line_info = &cs.base.line_info,
        .env = env,
    };

    evect_init(&exp_state.vec, strlen(str) + 1);
    struct keeper keeper = KEEPER(errcont->keeper);

    struct wlexer wlexer;
    wlexer_init(&wlexer, &cs.base);

    if (setjmp(keeper.env)) {
        free(exp_state.vec.data);
        shraise(errcont, NULL);
    } else {
        struct errcont sub_errcont = ERRCONT(errcont->errman, &keeper);
        exp_state.errcont = &sub_errcont;
        expand_guarded(&exp_state, &wlexer);
    }

    evect_push(&exp_state.vec, '\0');
    return exp_state.vec.data;
}
