#include <nsh_exec/environment.h>
#include <nsh_exec/runtime_error.h>
#include <nsh_utils/evect.h>
#include <nsh_utils/strutils.h>
#include <nsh_lex/wlexer.h>
#include <nsh_lex/lexer.h>
#include <nsh_lex/variable.h>
#include <nsh_exec/glob.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdarg.h>

#include "arithmetic_expansion.h"
#include "expansion.h"
#include "../error_compat.h"

static nsh_err_t expand_internal(struct expansion_state *exp_state,
                                 struct wlexer *wlexer) __unused_result;

nsh_err_t __unused_result expansion_error(struct expansion_state *exp_state,
                                          const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    lineinfo_vwarn(exp_state->line_info, fmt, ap);

    va_end(ap);

    expansion_state_env(exp_state)->code = 1;
    return NSH_EXECUTION_ERROR;
}

void expansion_warning(struct expansion_state *exp_state, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    lineinfo_vwarn(exp_state->line_info, fmt, ap);

    va_end(ap);
}

nsh_err_t expansion_end_word(struct expansion_state *exp_state)
{
    if (!expansion_callback_ctx_available(&exp_state->callback_ctx))
        return NSH_OK;

    expansion_result_push(&exp_state->result, '\0', 0);
    nsh_err_t err =
        glob_expand(&exp_state->glob_state, &exp_state->result, &exp_state->callback_ctx);
    expansion_state_reset_data(exp_state);
    return err;
}

enum expansion_sep_type
{
    EXPANSION_SEP_NONE = 0,
    EXPANSION_SEP_REGULAR,
    EXPANSION_SEP_SPACE,
};


static enum expansion_sep_type expansion_separator(struct expansion_state *exp_state,
                                                   char c)
{
    if (!char_bitset_get(&exp_state->ifs, c))
        return EXPANSION_SEP_NONE;

    if (isspace(c))
        return EXPANSION_SEP_SPACE;
    return EXPANSION_SEP_REGULAR;
}

/* push c into the result without going through IFS splitting */
void expansion_push_nosplit(struct expansion_state *exp_state, char c)
{
    expansion_result_push(&exp_state->result, c, expansion_is_unquoted(exp_state));
}

void expansion_push_nosplit_string(struct expansion_state *exp_state, const char *str)
{
    for (; *str; str++)
        expansion_push_nosplit(exp_state, *str);
}

/* push c into the result, going through IFS splitting
   beware: when splitting occurs, the callback can raise an exception */
nsh_err_t expansion_push_splitable(struct expansion_state *exp_state, char c)
{
    // IFS splitting is disabled inside quotes
    if (expansion_is_quoted(exp_state)) {
        expansion_push_nosplit(exp_state, c);
        return NSH_OK;
    }

    switch (expansion_separator(exp_state, c)) {
    case EXPANSION_SEP_NONE:
        expansion_push_nosplit(exp_state, c);
        return NSH_OK;
    case EXPANSION_SEP_REGULAR:
        if (exp_state->space_delimited) {
            // some space delimiters caused the word to be delimited.
            // IFS=' z'; var='a  zz'; printf '>%s<\n'
            exp_state->space_delimited = false;
            return NSH_OK;
        }

        return expansion_end_word(exp_state);
    case EXPANSION_SEP_SPACE:
        if (expansion_has_content(exp_state)) {
            exp_state->space_delimited = true;
            return expansion_end_word(exp_state);
        }
        return NSH_OK;
    }
}

nsh_err_t expansion_push_splitable_string(struct expansion_state *exp_state,
                                          const char *str)
{
    nsh_err_t err;
    for (; *str; str++)
        if ((err = expansion_push_splitable(exp_state, *str)))
            return err;
    return NSH_OK;
}


static int expand_internal(struct expansion_state *exp_state, struct wlexer *wlexer);

typedef int (*f_expander)(struct expansion_state *exp_state, struct wlexer *wlexer,
                          struct wtoken *wtoken);

static int expand_dollar(struct expansion_state *exp_state, struct wlexer *wlexer,
                         struct wtoken *wtoken)
{
    int rc;
    struct wlexer exp_wlexer = WLEXER_FORK(wlexer, MODE_EXPANSION);
    variable_name_reset(&exp_state->scratch_variable_name);

    do {
        memset(wtoken, 0, sizeof(*wtoken));
        wlexer_pop(wtoken, &exp_wlexer);
        if (wtoken->type == WTOK_EXP_CLOSE)
            break;

        if (wtoken->type != WTOK_REGULAR)
            return expansion_error(exp_state, "invalid character type in ${} section: %s",
                                   wtoken_type_to_string(wtoken->type));

        char c = wtoken->ch[0];
        if (!variable_name_check(&exp_state->scratch_variable_name, c))
            return expansion_error(exp_state,
                                   "invalid character in ${} section: `%c' (%#x)", c, c);

        variable_name_push(&exp_state->scratch_variable_name, c);
    } while (true);

    if (variable_name_size(&exp_state->scratch_variable_name) == 0)
        return expansion_error(exp_state, "empty parameter substitution");

    variable_name_finalize(&exp_state->scratch_variable_name);
    if ((rc = expand_name(exp_state,
                          variable_name_data(&exp_state->scratch_variable_name)))
        < 0)
        return rc;
    return LEXER_OP_CONTINUE;
}

static int expand_variable(struct expansion_state *exp_state, struct wlexer *wlexer,
                           struct wtoken *wtoken)
{
    int rc;

    // fetch the longest valid variable name
    variable_name_reset(&exp_state->scratch_variable_name);
    do {
        struct wtoken next_tok;
        wlexer_peek(&next_tok, wlexer);
        if (next_tok.type != WTOK_REGULAR)
            break;
        if (!variable_name_check(&exp_state->scratch_variable_name, next_tok.ch[0]))
            break;
        variable_name_push(&exp_state->scratch_variable_name, next_tok.ch[0]);
        wlexer_clear_lookahead(wlexer);
    } while (true);

    // push lonely dollars as is
    if (variable_name_size(&exp_state->scratch_variable_name) == 0) {
        expansion_push_nosplit(exp_state, wtoken->ch[0]);
        return LEXER_OP_CONTINUE;
    }

    // add the trailing null byte
    variable_name_finalize(&exp_state->scratch_variable_name);

    // look for the variable value, transfering the ownership of var_name
    if ((rc = expand_name(exp_state,
                          variable_name_data(&exp_state->scratch_variable_name)))
        < 0)
        return rc;
    return LEXER_OP_CONTINUE;
}

static int expand_tilde(struct expansion_state *exp_state, struct wlexer *wlexer,
                        struct wtoken *wtoken)
{
    /* push a tilde */
    expansion_push_nosplit(exp_state, '~');
    size_t start_offset = expansion_result_size(&exp_state->result);
    do {
        memset(wtoken, 0, sizeof(*wtoken));
        wlexer_peek(wtoken, wlexer);
        if (wtoken->type != WTOK_REGULAR)
            break;
        if (!portable_filename_char(wtoken->ch[0]))
            break;

        expansion_push_nosplit(exp_state, wtoken->ch[0]);
        wlexer_discard(wlexer);
    } while (true);
    size_t end_offset = expansion_result_size(&exp_state->result);

    const char *home;

    size_t username_length = end_offset - start_offset;
    if (username_length == 0) {
        /* no username case*/
        home = environment_var_get_cstring(expansion_state_env(exp_state), "HOME");
    } else {
        /* some username was provided */
        /* add a temporary nul byte (not accounted in size) */
        evect_finalize(&exp_state->result.string);
        char *username = expansion_result_data(&exp_state->result) + start_offset;
        struct passwd *passwd = getpwnam(username);
        if (passwd == NULL)
            return LEXER_OP_CONTINUE;

        home = passwd->pw_dir;
    }

    /* preserve the data if there's no home directory */
    if (home == NULL)
        return LEXER_OP_CONTINUE;

    expansion_result_cut(&exp_state->result, start_offset - 1);
    expansion_push_nosplit_string(exp_state, home);
    return LEXER_OP_CONTINUE;
}

static int expand_regular(struct expansion_state *exp_state, struct wlexer *wlexer,
                          struct wtoken *wtoken)
{
    nsh_err_t err;

    /* handle special meaning of regular characters */
    switch (wtoken->ch[0]) {
    case '~':
        /* ~ must be unquoted */
        if (wlexer->mode != MODE_UNQUOTED)
            break;

        /* expand if the tilde is at the start of the line */
        if (wlexer->cs->offset == 1)
            return expand_tilde(exp_state, wlexer, wtoken);

        /* or right after a colon during an assignment */
        if ((exp_state->flags & EXP_FLAGS_ASSIGNMENT)
            && expansion_result_last(&exp_state->result) == ':')
            return expand_tilde(exp_state, wlexer, wtoken);
        break;
    }

    // litteral regular characters from the unquoted mode
    // don't get any IFS splitting
    if (wlexer->mode == MODE_UNQUOTED) {
        expansion_push_nosplit(exp_state, wtoken->ch[0]);
        return LEXER_OP_CONTINUE;
    }

    if ((err = expansion_push_splitable(exp_state, wtoken->ch[0])))
        return err;
    return LEXER_OP_CONTINUE;
}

static int expand_eof(struct expansion_state *exp_state, struct wlexer *wlexer __unused,
                      struct wtoken *wtoken __unused)
{
    if (wlexer->mode != MODE_UNQUOTED)
        // TODO: display the current mode
        return expansion_error(exp_state, "EOF while expecting quote during expansion");
    return LEXER_OP_RETURN;
}

static int expand_squote(struct expansion_state *exp_state, struct wlexer *wlexer,
                         struct wtoken *wtoken __unused)
{
    int rc;
    if (wlexer->mode == MODE_SINGLE_QUOTED)
        return LEXER_OP_RETURN;

    exp_state->allow_empty_word = true;
    enum expansion_quoting prev_mode =
        expansion_switch_quoting(exp_state, EXPANSION_QUOTING_QUOTED);
    if ((rc = expand_internal(exp_state, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED))) < 0)
        return rc;
    exp_state->quoting_mode = prev_mode;
    return LEXER_OP_CONTINUE;
}

static int expand_dquote(struct expansion_state *exp_state __unused,
                         struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    int rc;
    if (wlexer->mode == MODE_DOUBLE_QUOTED)
        return LEXER_OP_RETURN;

    exp_state->allow_empty_word = true;
    enum expansion_quoting prev_mode =
        expansion_switch_quoting(exp_state, EXPANSION_QUOTING_QUOTED);
    if ((rc = expand_internal(exp_state, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED))) < 0)
        return rc;
    exp_state->quoting_mode = prev_mode;
    return LEXER_OP_CONTINUE;
}

static int expand_btick(struct expansion_state *exp_state, struct wlexer *wlexer,
                        struct wtoken *wtoken __unused)
{
    int rc;
    struct evect btick_content;
    evect_init(&btick_content, 32); // hopefuly sane default :(
    while (true) {
        memset(wtoken, 0, sizeof(*wtoken));
        wlexer_pop(wtoken, wlexer);
        if (wtoken->type == WTOK_EOF)
            return expansion_error(exp_state,
                                   "unexpected EOF in ` section, during expansion");

        // discard the final backtick
        if (wtoken->type == WTOK_BTICK)
            break;

        // inside a bticked section, escaped characters get unescaped
        // so their special meaning is restored inside the subshell
        if (wtoken->type == WTOK_ESCAPE) {
            evect_push(&btick_content, wtoken->ch[1]);
            continue;
        }

        evect_push_string(&btick_content, wtoken->ch);
    }

    evect_push(&btick_content, '\0');
    if ((rc = expand_subshell(exp_state, btick_content.data)) < 0)
        return rc;
    btick_content.data = NULL; // the ownership was transfered to expand_subshell
    return LEXER_OP_CONTINUE;
}

static int expand_escape(struct expansion_state *exp_state, struct wlexer *wlexer,
                         struct wtoken *wtoken __unused)
{
    int rc;
    // escapes have a special meaning inside backticks, but this is direcly
    // handled inside expand_btick

    int ch = wtoken->ch[1];

    /* handle PS1 / PS2 escapes */
    if ((exp_state->flags & EXP_FLAGS_PROMPT)) {
        if ((rc = expand_prompt_escape(exp_state, wlexer, ch)) < 0)
            return rc;
        if (rc != LEXER_OP_FALLTHROUGH)
            return rc;
    }

    // litteral regular characters from the unquoted mode
    // don't get any IFS splitting
    if (wlexer->mode == MODE_UNQUOTED) {
        expansion_push_nosplit(exp_state, ch);
        return LEXER_OP_CONTINUE;
    }

    if ((rc = expansion_push_splitable(exp_state, ch)) < 0)
        return rc;
    return LEXER_OP_CONTINUE;
}

static int expand_exp_subshell_open(struct expansion_state *exp_state,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    int rc;

    // run the lexer to find the end of the subshell
    struct wlexer sub_wlexer = WLEXER_FORK(wlexer, MODE_SUBSHELL);
    char *subshell_content;
    if ((rc = lexer_lex_string(&subshell_content, &sub_wlexer)) < 0)
        return rc;

    // perform the expansion
    if ((rc = expand_subshell(exp_state, subshell_content)) < 0)
        return rc;

    subshell_content = NULL; // the ownership was transfered to expand_subshell
    return LEXER_OP_CONTINUE;
}

static int expand_arith_open(struct expansion_state *exp_state, struct wlexer *wlexer,
                             struct wtoken *wtoken __unused)
{
    // switch to nosplit quoted mode to avoid buffer flushes
    enum expansion_quoting prev_mode =
        expansion_switch_quoting(exp_state, EXPANSION_QUOTING_NOSPLIT);

    int rc;

    // expand the content of the arithmetic expansion
    int initial_size = expansion_result_size(&exp_state->result);
    if ((rc = expand_internal(exp_state, &WLEXER_FORK(wlexer, MODE_ARITH))))
        return rc;

    // get the arithmetic expression in a single string
    expansion_push_nosplit(exp_state, '\0');
    char *arith_content = expansion_result_data(&exp_state->result) + initial_size;

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
        return rc;

    struct arith_token next_tok;
    if ((rc = arith_lexer_peek(&next_tok, &alexer)))
        // this line may never run at all as the parser already peeks the next token
        return expansion_error(exp_state, "arithmetic post-parsing token peek failed");

    if (next_tok.type != &arith_type_eof)
        return expansion_error(exp_state, "unexpected token: %s", next_tok.type->name);

    // convert the value to an integer
    int res = arith_value_to_int(exp_state, &res_val);
    arith_value_destroy(&res_val);

    char print_buf[INT_MAX_CHARS(int) + /* \0 */ 1];
    sprintf(print_buf, "%d", res);

    // reset the expansion buffer to its starting point
    expansion_result_cut(&exp_state->result, initial_size);

    // push the result of the arithmetic expansion in the expansion buffer
    if ((rc = expansion_push_splitable_string(exp_state, print_buf)) < 0)
        return rc;
    exp_state->quoting_mode = prev_mode;
    return LEXER_OP_CONTINUE;
}

static int expand_arith_close(struct expansion_state *exp_state __unused,
                              struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH);
    return LEXER_OP_RETURN;
}

static int expand_invalid_state(struct expansion_state *exp_state,
                                struct wlexer *wlexer __unused,
                                struct wtoken *wtoken __unused)
{
    return expansion_error(exp_state, "reached an invalid state during expansion");
}

static int expand_arith_group_open(struct expansion_state *exp_state,
                                   struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    int rc;
    assert(wlexer->mode == MODE_ARITH_GROUP || wlexer->mode == MODE_ARITH);
    // expand to the litteral value, so that the arithmetic interpreter can
    // re-parse it after parameter substitution
    expansion_push_nosplit(exp_state, '(');
    if ((rc = expand_internal(exp_state, &WLEXER_FORK(wlexer, MODE_ARITH_GROUP))) < 0)
        return rc;
    expansion_push_nosplit(exp_state, ')');
    return LEXER_OP_CONTINUE;
}

static int expand_arith_group_close(struct expansion_state *exp_state __unused,
                                    struct wlexer *wlexer, struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH_GROUP);
    return LEXER_OP_RETURN;
}

static f_expander expanders[] = {
    [WTOK_EOF] = expand_eof,
    [WTOK_REGULAR] = expand_regular,
    [WTOK_VARIABLE] = expand_variable,
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

static nsh_err_t expand_internal(struct expansion_state *exp_state, struct wlexer *wlexer)
{
    int rc;
    while (true) {
        // read a word token
        struct wtoken wtoken;
        wlexer_pop(&wtoken, wlexer);

        // react depending on the token type
        if ((rc = expanders[wtoken.type](exp_state, wlexer, &wtoken)) < 0)
            return rc;
        assert(rc != LEXER_OP_FALLTHROUGH && rc != LEXER_OP_PUSH);
        if (rc & LEXER_OP_RETURN)
            return NSH_OK;
    }
}

nsh_err_t expand(struct expansion_state *exp_state, struct wlexer *wlexer)
{
    nsh_err_t err;

    if ((err = expand_internal(exp_state, wlexer)))
        return err;

    /* push the last section when using IFS splitting. */
    if (expansion_has_content(exp_state))
        if ((err = expansion_end_word(exp_state)))
            return err;

    return NSH_OK;
}

nsh_err_t expand_nosplit(char **res, struct lineinfo *line_info, const char *str,
                         struct environment *env, int flags)
{
    nsh_err_t err;

    /* initialize the character stream */
    struct cstream_string cs;
    cstream_string_init(&cs, str);
    cs.base.line_info = LINEINFO("<expansion>", line_info);

    /* perform the recursive expansion */
    struct wlexer wlexer;
    wlexer_init(&wlexer, &cs.base);

    /* initialize the expansion buffer */
    struct expansion_state exp_state;
    expansion_state_init(&exp_state, EXPANSION_QUOTING_NOSPLIT, flags);
    expansion_callback_ctx_init(&exp_state.callback_ctx, NULL, env);
    exp_state.line_info = &cs.base.line_info;

    /* perform the expansion */
    if ((err = expand(&exp_state, &wlexer)))
        goto err_expand;

    /* steal the content of the result data buffer */
    struct evect res_buf;
    evect_steal(&exp_state.result.string, &res_buf);

    /* cleanup the expansion state */
    expansion_state_destroy(&exp_state);

    /* finalize the buffer and return it */
    evect_push(&res_buf, '\0');
    *res = evect_data(&res_buf);
    return err;

err_expand:
    expansion_state_destroy(&exp_state);
    return err;
}


char *expand_nosplit_exception(struct lineinfo *line_info, const char *str, int flags,
                               struct environment *env, struct exception_catcher *catcher)
{
    char *res;
    nsh_err_t err;
    if ((err = expand_nosplit(&res, line_info, str, env, flags)))
        raise_from_error(catcher, err);
    return res;
}


static nsh_err_t expand_word_callback(struct expansion_callback *callback,
                                      struct shword *word, struct environment *env,
                                      int flags)
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
    expansion_state_init(&exp_state, EXPANSION_QUOTING_UNQUOTED, flags);
    expansion_callback_ctx_init(&exp_state.callback_ctx, callback, env);
    exp_state.line_info = &cs.base.line_info;

    /* IFS is reduced into a bitset, as it could be modified during expansion */
    const char *cur_ifs = environment_var_get_cstring(env, "IFS");
    expansion_state_set_field_sep(&exp_state, cur_ifs);

    /* perform the expansion */
    nsh_err_t err = expand(&exp_state, &wlexer);

    /* cleanup the expansion state */
    expansion_state_destroy(&exp_state);

    return err;
}

nsh_err_t expand_wordlist_callback(struct expansion_callback *callback,
                                   struct wordlist *wl, struct environment *env,
                                   int flags)
{
    nsh_err_t err;
    for (size_t i = 0; i < wordlist_size(wl); i++)
        if ((err = expand_word_callback(callback, wordlist_get(wl, i), env, flags)))
            return err;
    return NSH_OK;
}

static nsh_err_t expansion_word_callback(void *data, char *word,
                                         struct environment *env __unused)
{
    struct cpvect *res = data;
    cpvect_push(res, word);
    return NSH_OK;
}

nsh_err_t expand_wordlist(struct cpvect *res, struct wordlist *wl,
                          struct environment *env, int flags)
{
    struct expansion_callback callback = {
        .func = expansion_word_callback,
        .data = res,
    };
    return expand_wordlist_callback(&callback, wl, env, flags);
}
