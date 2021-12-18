#include <nsh_lex/wlexer.h>
#include <nsh_utils/macros.h>
#include <nsh_utils/logging.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>


static int wlexer_lex(struct wtoken *res, struct wlexer *lex);

static int ensure_lookahead(struct wlexer *lex)
{
    if (wlexer_has_lookahead(lex))
        return NSH_OK;
    return wlexer_lex(&lex->lookahead, lex);
}

int wlexer_peek(struct wtoken *res, struct wlexer *lex)
{
    int rc;
    if ((rc = ensure_lookahead(lex)))
        return rc;

    *res = lex->lookahead;
    return NSH_OK;
}

int wlexer_peek_type(struct wlexer *lex)
{
    int rc;
    if ((rc = ensure_lookahead(lex)))
        return rc;

    return lex->lookahead.type;
}


void wlexer_discard(struct wlexer *lex)
{
    assert(wlexer_has_lookahead(lex));
    wlexer_clear_lookahead(lex);
}

int wlexer_pop(struct wtoken *res, struct wlexer *lex)
{
    if (!wlexer_has_lookahead(lex))
        return wlexer_lex(res, lex);

    *res = lex->lookahead;
    wlexer_clear_lookahead(lex);
    return NSH_OK;
}

void wlexer_push(const struct wtoken *res, struct wlexer *lex)
{
    assert(!wlexer_has_lookahead(lex));
    lex->lookahead = *res;
    return;
}


struct wlexer_rule
{
    char pattern[4];
    enum wtoken_type type;
    enum wlexer_mode valid_modes;
};


#define EXP_MODES ~MODE_SINGLE_QUOTED
static struct wlexer_rule rules[] = {
    {"'", WTOK_SQUOTE, ~MODE_DOUBLE_QUOTED},
    {"`", WTOK_BTICK, ~MODE_SINGLE_QUOTED},
    {"\"", WTOK_DQUOTE, ~MODE_SINGLE_QUOTED},
    {"}", WTOK_EXP_CLOSE, MODE_EXPANSION},
    {"${", WTOK_EXP_OPEN, EXP_MODES},
    {"$((", WTOK_ARITH_OPEN, EXP_MODES},
    {"))", WTOK_ARITH_CLOSE, MODE_ARITH},
    {"$(", WTOK_EXP_SUBSH_OPEN, EXP_MODES},
    {")", WTOK_EXP_SUBSH_CLOSE, MODE_EXP_SUBSHELL},
    {"$", WTOK_VARIABLE, EXP_MODES},
    {"(", WTOK_ARITH_GROUP_OPEN, MODE_ARITH | MODE_ARITH_GROUP},
    {")", WTOK_ARITH_GROUP_CLOSE, MODE_ARITH_GROUP},
    {"(", WTOK_SUBSH_OPEN, MODE_UNQUOTED | MODE_SUBSHELL | MODE_EXP_SUBSHELL},
    {")", WTOK_SUBSH_CLOSE, MODE_UNQUOTED | MODE_SUBSHELL},
};


static bool char_starts_rule(enum wlexer_mode mode, char c)
{
    for (size_t i = 0; i < ARR_SIZE(rules); i++) {
        if (!(rules[i].valid_modes & mode))
            continue;
        if (rules[i].pattern[0] == c)
            return true;
    }
    return false;
}


static int match_rule(struct wtoken *tok, struct wlexer *lex, struct wlexer_rule *rule)
{
    int rc;

    if (!(rule->valid_modes & lex->mode))
        return false;

    for (int i = 0;; i++) {
        // if the end of the pattern is reached, it matched
        unsigned char pat_char = rule->pattern[i];
        if (pat_char == '\0')
            return true;

        // if there are no more characters in the buffer,
        // only pull the next character if it matches
        if (tok->ch[i] == '\0') {
            assert(i < 4);
            if ((rc = cstream_peek(lex->cs)) < 0)
                return rc;
            int next_char = rc;
            if (next_char == CSTREAM_EOF)
                return false;
            if (next_char != pat_char)
                return false;
            if ((rc = cstream_pop(lex->cs)) < 0)
                return rc;
            tok->ch[i] = rc;
        }

        if (tok->ch[i] != pat_char)
            return false;
    }
}

static int wlexer_lex_escape(struct wtoken *res, struct wlexer *lex)
{
    int rc;

    if (lex->mode == MODE_SINGLE_QUOTED)
        goto regular;

    // "A <backslash> that is not quoted shall preserve the literal value of
    // the following character, with the exception of a <newline>"
    // skipping newlines is done in the lexer
    if (lex->mode == MODE_UNQUOTED)
        goto escape;

    if ((rc = cstream_peek(lex->cs)) < 0)
        return rc;
    int next_char = rc;
    if (next_char == CSTREAM_EOF)
        // TODO: raise an error when this occurs
        goto escape;

    if (next_char == '\\')
        goto escape;

    // backticked sections have weird escaping rules
    if (lex->mode == MODE_BTICK) {
        // Within the backquoted style of command substitution,
        // <backslash> shall retain its literal meaning, except
        // when followed by: '$', '`', or <backslash>
        if (next_char == '$' || next_char == '`')
            goto escape;
    } else {
        if (char_starts_rule(lex->mode, next_char))
            goto escape;
    }


regular:
    res->type = WTOK_REGULAR;
    return NSH_OK;

escape:
    res->type = WTOK_ESCAPE;
    if ((rc = cstream_pop(lex->cs)) < 0)
        return rc;
    res->ch[1] = rc;
    return NSH_OK;
}

static int wlexer_lex(struct wtoken *res, struct wlexer *lex)
{
    int rc;

    memset(res->ch, 0, sizeof(res->ch));
    res->type = WTOK_UNKNOWN;

    if ((rc = cstream_pop(lex->cs)) < 0)
        return rc;
    if (rc == CSTREAM_EOF) {
        res->type = WTOK_EOF;
        goto lexing_done;
    }
    res->ch[0] = rc;

    if (res->ch[0] == '\\') {
        if ((rc = wlexer_lex_escape(res, lex)) < 0)
            return rc;
        assert(res->type != WTOK_UNKNOWN);
        goto lexing_done;
    }

    for (size_t i = 0; i < ARR_SIZE(rules); i++) {
        struct wlexer_rule *cur_rule = &rules[i];
        if ((rc = match_rule(res, lex, cur_rule)) < 0)
            return rc;
        if (rc) {
            res->type = cur_rule->type;
            goto lexing_done;
        }
    }

    res->type = WTOK_REGULAR;
lexing_done:
    nsh_debug("wtoken { type: %-10s repr: '%s' }", wtoken_type_to_string(res->type),
              wtoken_repr_data(res));
    return NSH_OK;
}


const char *wtoken_repr_data(struct wtoken *tok)
{
    if (tok->type == WTOK_UNKNOWN)
        return "<unknown>";
    if (tok->type == WTOK_EOF)
        return "<EOF>";
    if (tok->type == WTOK_REGULAR && tok->ch[0] == '\n')
        return "<newline>";
    if (tok->type == WTOK_ESCAPE && tok->ch[1] == '\n')
        return "<escaped newline>";
    return tok->ch;
}


const char *wtoken_type_to_string(enum wtoken_type type)
{
    switch (type) {
    case WTOK_UNKNOWN:
        return "unknown";
    case WTOK_SQUOTE:
        return "simple quote";
    case WTOK_DQUOTE:
        return "double quote";
    case WTOK_BTICK:
        return "backtick";
    case WTOK_ESCAPE:
        return "escape";
    case WTOK_EOF:
        return "EOF";
    case WTOK_EXP_SUBSH_OPEN:
        return "expansion subshell open";
    case WTOK_EXP_SUBSH_CLOSE:
        return "expansion subshell close";
    case WTOK_SUBSH_OPEN:
        return "subshell open";
    case WTOK_SUBSH_CLOSE:
        return "subshell close";
    case WTOK_ARITH_OPEN:
        return "arithmetic open";
    case WTOK_ARITH_CLOSE:
        return "arithmetic clone";
    case WTOK_ARITH_GROUP_OPEN:
        return "arithmetic group open";
    case WTOK_ARITH_GROUP_CLOSE:
        return "arithmetic group close";
    case WTOK_EXP_OPEN:
        return "expansion open";
    case WTOK_EXP_CLOSE:
        return "expansion close";
    case WTOK_REGULAR:
        return "regular";
    case WTOK_VARIABLE:
        return "variable";
    }
    abort();
}
