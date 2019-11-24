#include "shexec/builtins.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/evect.h"
#include "utils/strutils.h"
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

static void __noreturn expansion_raise(struct expansion_state *exp_state)
{
    shraise(exp_state->errcont, &g_lexer_error);
}

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

void expansion_end_word(struct expansion_state *exp_state)
{
    if (exp_state->callback.func == NULL)
        return;

    exp_state->callback.func(exp_state, exp_state->callback.data);
    expansion_state_reset_data(exp_state);
}

enum expansion_sep_type
{
    EXPANSION_SEP_NONE = 0,
    EXPANSION_SEP_REGULAR,
    EXPANSION_SEP_SPACE,
};

static enum expansion_sep_type expansion_separator(struct expansion_state *exp_state, char c)
{
    if (exp_state->IFS == NULL)
        return EXPANSION_SEP_NONE;

    for (const char *sep = exp_state->IFS; *sep; sep++)
        if (*sep == c) {
            if (isspace(c))
                return EXPANSION_SEP_SPACE;
            return EXPANSION_SEP_REGULAR;
        }
    return EXPANSION_SEP_NONE;
}

void expansion_push_nosplit(struct expansion_state *exp_state, char c)
{

    evect_push(&exp_state->result.string, c);
    evect_push(&exp_state->result.metadata, expansion_is_unquoted(exp_state));
}

// handle IFS splitting of a given character
void expansion_push(struct expansion_state *exp_state, char c)
{
    // IFS splitting is disabled inside quotes
    if (expansion_is_quoted(exp_state)) {
        expansion_push_nosplit(exp_state, c);
        return;
    }

    switch (expansion_separator(exp_state, c)) {
    case EXPANSION_SEP_NONE:
        expansion_push_nosplit(exp_state, c);
        return;
    case EXPANSION_SEP_REGULAR:
        if (exp_state->space_delimited) {
            // some space delimiters caused the word to be delimited.
            // IFS=' z'; var='a  zz'; printf '>%s<\n'
            exp_state->space_delimited = false;
            return;
        }

        expansion_end_word(exp_state);
        return;
    case EXPANSION_SEP_SPACE:
        if (expansion_has_content(exp_state)) {
            exp_state->space_delimited = true;
            expansion_end_word(exp_state);
        }
        return;
    }
}

void expansion_push_string(struct expansion_state *exp_state, const char *str)
{
    for (; *str; str++)
        expansion_push(exp_state, *str);
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
    expand_name(exp_state, var_name.simple_var.data);
    variable_name_destroy(&var_name);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_variable(struct expansion_state *exp_state,
                                      struct wlexer *wlexer,
                                      struct wtoken *wtoken)
{
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
        expansion_push(exp_state, wtoken->ch[0]);
        return LEXER_OP_CONTINUE;
    }

    // add the trailing null byte
    variable_name_finalize(&var_name);

    // look for the variable value
    expand_name(exp_state, variable_name_data(&var_name));

    variable_name_destroy(&var_name);
    return LEXER_OP_CONTINUE;
}


static enum wlexer_op expand_regular(struct expansion_state *exp_state,
                                     struct wlexer *wlexer,
                                     struct wtoken *wtoken)
{
    if (wlexer->mode != MODE_SINGLE_QUOTED && wtoken->ch[0] == '$')
        return expand_variable(exp_state, wlexer, wtoken);

    // litteral regular characters from the unquoted mode
    // don't get any IFS splitting
    if (wlexer->mode == MODE_UNQUOTED) {
        expansion_push_nosplit(exp_state, wtoken->ch[0]);
        return LEXER_OP_CONTINUE;
    }

    expansion_push(exp_state, wtoken->ch[0]);
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

static enum wlexer_op expand_squote(struct expansion_state *exp_state,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    if (wlexer->mode == MODE_SINGLE_QUOTED)
        return LEXER_OP_RETURN;

    exp_state->allow_empty_word = true;
    expansion_end_section(exp_state);
    enum expansion_quoting prev_mode = expansion_switch_quoting(exp_state, EXPANSION_QUOTING_QUOTED);
    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED));
    exp_state->quoting_mode = prev_mode;
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_dquote(struct expansion_state *exp_state __unused,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    if (wlexer->mode == MODE_DOUBLE_QUOTED)
        return LEXER_OP_RETURN;

    exp_state->allow_empty_word = true;
    expansion_end_section(exp_state);
    enum expansion_quoting prev_mode = expansion_switch_quoting(exp_state, EXPANSION_QUOTING_QUOTED);
    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED));
    exp_state->quoting_mode = prev_mode;
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_btick(struct expansion_state *exp_state,
                                   struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    expansion_end_section(exp_state);

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
    expand_subshell(exp_state, btick_content.data);
    btick_content.data = NULL; // the ownership was transfered to expand_subshell
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_escape(struct expansion_state *exp_state,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    expansion_end_section(exp_state);

    // clearing characters isn't safe if
    // the wlexer has some cached tokens
    assert(!wlexer_has_lookahead(wlexer));
    int ch = cstream_pop(wlexer->cs);
    if (ch == EOF)
        expansion_error(exp_state, "unexpected EOF in escape, during expansion");

    expansion_push(exp_state, ch);
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_exp_subshell_open(struct expansion_state *exp_state,
                                               struct wlexer *wlexer,
                                               struct wtoken *wtoken __unused)
{
    expansion_end_section(exp_state);

    struct wlexer sub_wlexer = WLEXER_FORK(wlexer, MODE_SUBSHELL);
    char *subshell_content = lexer_lex_string(exp_state->errcont, &sub_wlexer);
    expand_subshell(exp_state, subshell_content);
    subshell_content = NULL; // the ownership was transfered to expand_subshell
    return LEXER_OP_CONTINUE;
}

static enum wlexer_op expand_arith_open(struct expansion_state *exp_state,
                                        struct wlexer *wlexer,
                                        struct wtoken *wtoken __unused)
{
    expansion_end_section(exp_state);

    // switch to nosplit quoted mode to avoid buffer flushes
    enum expansion_quoting prev_mode = expansion_switch_quoting(exp_state, EXPANSION_QUOTING_NOSPLIT);

    int rc;

    // expand the content of the arithmetic expansion
    int initial_size = expansion_result_size(&exp_state->result);
    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_ARITH));

    // get the arithmetic expression in a single string
    expansion_push(exp_state, '\0');
    char *arith_content = evect_data(&exp_state->result.string) + initial_size;

    // prepare the arithmetic wlexer and stream
    struct cstream_string cs;
    cstream_string_init(&cs, arith_content);
    cs.base.line_info = LINEINFO("<arithmetics>", exp_state->line_info);

    struct arith_lexer alexer = {
        .exp_state = exp_state,
        .cs = &cs.base,
    };

    struct arith_value res_val;
    if ((rc = arith_parse(&res_val, &alexer, 0)))
        expansion_raise(exp_state);

    struct arith_token next_tok;
    if ((rc = arith_lexer_peek(&next_tok, &alexer)))
        // this line may never run at all as the parser already peeks the next token
        expansion_error(exp_state, "arithmetic post-parsing token peek failed");

    if (next_tok.type != &arith_type_eof)
        expansion_error(exp_state, "unexpected token: %s", next_tok.type->name);

    // convert the value to an integer
    int res = arith_value_to_int(exp_state, &res_val);
    char print_buf[INT_MAX_CHARS(int) + /* \0 */ 1];
    sprintf(print_buf, "%d", res);

    // reset the expansion buffer to its starting point
    expansion_result_cut(&exp_state->result, initial_size);

    // push the result of the arithmetic expansion in the expansion buffer
    expansion_push_string(exp_state, print_buf);
    exp_state->quoting_mode = prev_mode;
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
    expansion_push(exp_state, '(');
    expand_guarded(exp_state, &WLEXER_FORK(wlexer, MODE_ARITH_GROUP));
    expansion_push(exp_state, ')');
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

void expand(struct expansion_state *exp_state,
            struct wlexer *wlexer,
            struct errcont *errcont)
{
    // when IFS is NULL, callback is NULL too.
    // when isn't not NULL, there must be a callback
    assert((exp_state->callback.func == NULL) == (exp_state->IFS == NULL));

    /* on exception, free the expansion buffer */
    struct keeper keeper = KEEPER(errcont->keeper);
    struct errcont sub_errcont = ERRCONT(errcont->errman, &keeper);
    if (setjmp(keeper.env)) {
        expansion_result_destroy(&exp_state->result);
        shraise(errcont, NULL);
    }
    exp_state->errcont = &sub_errcont;
    expand_guarded(exp_state, wlexer);

    /* push the last section when using IFS splitting. */
    if (expansion_has_content(exp_state))
        expansion_end_word(exp_state);
}

char *expand_nosplit(struct lineinfo *line_info, char *str, struct environment *env, struct errcont *errcont)
{
    /* initialize the character stream */
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<expansion>", line_info);

    /* perform the recursive expansion */
    struct wlexer wlexer;
    wlexer_init(&wlexer, &cs.base);

    /* initialize the expansion buffer */
    struct expansion_state exp_state;
    expansion_state_init(&exp_state, env, EXPANSION_QUOTING_NOSPLIT);
    exp_state.line_info = &cs.base.line_info;

    /* perform the expansion */
    expand(&exp_state, &wlexer, errcont);

    /* finalize the buffer and return it */
    evect_destroy(&exp_state.result.metadata);
    evect_push(&exp_state.result.string, '\0');
    return evect_data(&exp_state.result.string);
}

static void expand_word_callback(struct expansion_callback *callback, struct shword *word, struct environment *env, struct errcont *errcont)
{
    /* initialize the character stream */
    struct cstream_string cs;
    cstream_string_init(&cs, shword_buf(word));
    cs.base.line_info = LINEINFO("<expansion>", shword_line_info(word));

    /* perform the recursive expansion */
    struct wlexer wlexer;
    wlexer_init(&wlexer, &cs.base);

    /* initialize the expansion buffer */
    struct expansion_state exp_state;
    expansion_state_init(&exp_state, env, EXPANSION_QUOTING_UNQUOTED);
    exp_state.line_info = &cs.base.line_info;
    exp_state.callback = *callback;
    exp_state.IFS = environment_var_get(env, "IFS");

    /* perform the expansion */
    expand(&exp_state, &wlexer, errcont);

    /* free the result buffer */
    expansion_result_destroy(&exp_state.result);
}

void expand_wordlist_callback(struct expansion_callback *callback, struct wordlist *wl, struct environment *env, struct errcont *errcont)
{
    for (size_t i = 0; i < wordlist_size(wl); i++)
        expand_word_callback(callback, wordlist_get(wl, i), env, errcont);
}

static void expansion_word_callback(struct expansion_state *exp_state, void *callback_data)
{
    struct cpvect *res = callback_data;
    char *data = evect_data(&exp_state->result.string);
    size_t size = expansion_result_size(&exp_state->result);
    cpvect_push(res, strndup(data, size));
}

void expand_wordlist(struct cpvect *res, struct wordlist *wl, struct environment *env, struct errcont *errcont)
{
    struct expansion_callback callback = {
        .func = expansion_word_callback,
        .data = res,
    };
    expand_wordlist_callback(&callback, wl, env, errcont);
}
