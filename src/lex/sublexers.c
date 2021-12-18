#include <nsh_lex/lexer.h>
#include <nsh_lex/variable.h>
#include <nsh_lex/wlexer.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/macros.h>

#include "lexer.h"


#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


static int sublexer_eof(struct lexer *lexer, struct wlexer *wlexer, struct token *token,
                        struct wtoken *wtoken __unused)
{
    if (wlexer->mode != MODE_UNQUOTED)
        return lexer_err(lexer, "EOF while expecting quote");
    if (token_size(token) == 0)
        token->type = TOK_EOF;
    return LEXER_OP_RETURN;
}

static int sublexer_squote(struct lexer *lexer, struct wlexer *wlexer,
                           struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    if (wlexer->mode == MODE_SINGLE_QUOTED)
        return LEXER_OP_RETURN;

    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SINGLE_QUOTED), lexer);
    return LEXER_OP_CONTINUE;
}

static int sublexer_dquote(struct lexer *lexer, struct wlexer *wlexer,
                           struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    if (wlexer->mode == MODE_DOUBLE_QUOTED)
        return LEXER_OP_RETURN;

    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_DOUBLE_QUOTED), lexer);
    return LEXER_OP_CONTINUE;
}


static int sublexer_btick(struct lexer *lexer __unused, struct wlexer *wlexer,
                          struct token *token, struct wtoken *wtok)
{
    int rc;
    wtoken_push(token, wtok);
    // escaping is directly handled by the word lexer
    do {
        memset(wtok, 0, sizeof(*wtok));
        if ((rc = wlexer_pop(wtok, wlexer)) < 0)
            return rc;
        if (wtok->type == WTOK_EOF)
            return lexer_err(lexer, "unexpected EOF in ` section");
        evect_push_string(&token->str, wtok->ch);
    } while (wtok->type != WTOK_BTICK);
    return LEXER_OP_CONTINUE;
}

static int sublexer_escape(struct lexer *lexer __unused, struct wlexer *wlexer __unused,
                           struct token *token, struct wtoken *wtoken)
{
    // ignore escaped carriage returns
    if (wtoken->ch[1] != '\n')
        evect_push_string(&token->str, wtoken->ch);
    return LEXER_OP_CONTINUE;
}

static int sublexer_exp_subsh_open(struct lexer *lexer, struct wlexer *wlexer,
                                   struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXP_SUBSHELL), lexer);
    struct wtoken end_wtoken = {
        .ch = {')'},
        .type = WTOK_EXP_SUBSH_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static int sublexer_exp_subsh_close(struct lexer *lexer __unused, struct wlexer *wlexer,
                                    struct token *token __unused,
                                    struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_EXP_SUBSHELL);
    return LEXER_OP_RETURN;
}

static int sublexer_subsh_open(struct lexer *lexer, struct wlexer *wlexer,
                               struct token *token __unused, struct wtoken *wtoken)
{
    /* (echo test) */
    /* ^           */
    if (wlexer->mode == MODE_UNQUOTED) {
        /* oops(echo test) */
        /*     ^           */
        if (token_size(token) != 0)
            /* break the first token appart */
            return LEXER_OP_CANCEL;
        wtoken_push(token, wtoken);
        token->type = TOK_LPAR;
        return LEXER_OP_RETURN;
    }

    /* $( (test)) */
    /*    ^       */
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_SUBSHELL), lexer);
    struct wtoken end_wtoken = {
        .ch = {')'},
        .type = WTOK_SUBSH_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static int sublexer_subsh_close(struct lexer *lexer __unused, struct wlexer *wlexer,
                                struct token *token __unused,
                                struct wtoken *wtoken __unused)
{
    /* (echo test) */
    /*           ^ */
    if (wlexer->mode == MODE_UNQUOTED) {
        /* oops(echo test) */
        /*               ^ */
        if (token_size(token) != 0)
            return LEXER_OP_CANCEL;

        wtoken_push(token, wtoken);
        token->type = TOK_RPAR;
        return LEXER_OP_RETURN;
    }

    assert(wlexer->mode == MODE_SUBSHELL || wlexer->mode == MODE_UNQUOTED);
    return LEXER_OP_RETURN;
}

static int sublexer_arith_open(struct lexer *lexer, struct wlexer *wlexer,
                               struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH), lexer);
    struct wtoken end_wtoken = {
        .ch = {')', ')'},
        .type = WTOK_ARITH_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static int sublexer_arith_close(struct lexer *lexer __unused, struct wlexer *wlexer,
                                struct token *token __unused,
                                struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH);
    return LEXER_OP_RETURN;
}

static int sublexer_exp_open(struct lexer *lexer, struct wlexer *wlexer,
                             struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_EXPANSION), lexer);
    struct wtoken end_wtoken = {
        .ch = {'}'},
        .type = WTOK_EXP_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static int sublexer_exp_close(struct lexer *lexer __unused, struct wlexer *wlexer,
                              struct token *token __unused,
                              struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_EXPANSION);
    return LEXER_OP_RETURN;
}

static int sublexer_arith_group_open(struct lexer *lexer, struct wlexer *wlexer,
                                     struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    lexer_lex_untyped(token, &WLEXER_FORK(wlexer, MODE_ARITH_GROUP), lexer);
    struct wtoken end_wtoken = {
        .ch = {')'},
        .type = WTOK_ARITH_GROUP_CLOSE,
    };
    wtoken_push(token, &end_wtoken);
    return LEXER_OP_CONTINUE;
}

static int sublexer_arith_group_close(struct lexer *lexer __unused, struct wlexer *wlexer,
                                      struct token *token __unused,
                                      struct wtoken *wtoken __unused)
{
    assert(wlexer->mode == MODE_ARITH_GROUP);
    return LEXER_OP_RETURN;
}

static int sublexer_variable(struct lexer *lexer __unused, struct wlexer *wlexer __unused,
                             struct token *token, struct wtoken *wtoken)
{
    wtoken_push(token, wtoken);
    return LEXER_OP_CONTINUE;
}


sublexer_f sublexers[] = {
    [WTOK_EOF] = sublexer_eof,
    [WTOK_REGULAR] = sublexer_regular,
    [WTOK_VARIABLE] = sublexer_variable,
    [WTOK_SQUOTE] = sublexer_squote,
    [WTOK_DQUOTE] = sublexer_dquote,
    [WTOK_BTICK] = sublexer_btick,
    [WTOK_ESCAPE] = sublexer_escape,
    [WTOK_EXP_SUBSH_OPEN] = sublexer_exp_subsh_open,
    [WTOK_EXP_SUBSH_CLOSE] = sublexer_exp_subsh_close,
    [WTOK_SUBSH_OPEN] = sublexer_subsh_open,
    [WTOK_SUBSH_CLOSE] = sublexer_subsh_close,
    [WTOK_ARITH_OPEN] = sublexer_arith_open,
    [WTOK_ARITH_CLOSE] = sublexer_arith_close,
    [WTOK_EXP_OPEN] = sublexer_exp_open,
    [WTOK_EXP_CLOSE] = sublexer_exp_close,
    [WTOK_ARITH_GROUP_OPEN] = sublexer_arith_group_open,
    [WTOK_ARITH_GROUP_CLOSE] = sublexer_arith_group_close,
};
