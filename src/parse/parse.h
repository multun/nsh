#pragma once

#include <nsh_parse/parse.h>
#include <nsh_parse/parser_error.h>

#include <stdarg.h>


/** This return value is used when a rule did not match */
#define PARSER_NOMATCH 1


int parser_err(const struct token *tok, const char *fmt, ...) __unused_result;
int parser_consume(struct lexer *lexer, enum token_type type) __unused_result;
int parser_consume_optional(struct lexer *lexer, enum token_type type) __unused_result;
int parse_newlines(struct lexer *lexer) __unused_result;
int parse_list(struct shast **res, struct lexer *lexer) __unused_result;
int parse_and_or(struct shast **res, struct lexer *lexer) __unused_result;
int parse_pipeline(struct shast **res, struct lexer *lexer) __unused_result;
int parse_command(struct shast **res, struct lexer *lexer) __unused_result;
int parse_simple_command(struct shast **res, struct lexer *lexer) __unused_result;
int parse_funcdec(struct shast **res, struct lexer *lexer) __unused_result;
int parse_compound_list(struct shast **res, struct lexer *lexer) __unused_result;
int parse_for(struct shast **res, struct lexer *lexer) __unused_result;
int parse_while(struct shast **res, struct lexer *lexer) __unused_result;
int parse_until(struct shast **res, struct lexer *lexer) __unused_result;
int parse_case(struct shast **res, struct lexer *lexer) __unused_result;
int parse_if(struct shast **res, struct lexer *lexer) __unused_result;
int parse_else_clause(struct shast **res, struct lexer *lexer) __unused_result;
int parse_do_group(struct shast **res, struct lexer *lexer) __unused_result;
int parse_word(struct shword **res, struct lexer *lexer) __unused_result;
int parse_redirection(struct shast_redirection **res,
                      struct lexer *lexer) __unused_result;
int parse_redirections(struct shast_block **block, struct shast ***res,
                       struct lexer *lexer) __unused_result;


static inline int parser_match(enum token_type tok_type, struct lexer *lexer)
{
    int rc;
    const struct token *tok;
    if ((rc = lexer_peek(&tok, lexer)))
        return rc;

    if (!token_is(tok, tok_type))
        return PARSER_NOMATCH;

    return NSH_OK;
}

static inline int parser_match_discard(enum token_type tok_type, struct lexer *lexer)
{
    int rc;
    if ((rc = parser_match(tok_type, lexer)))
        return rc;

    if ((rc = lexer_discard(lexer)))
        return rc;

    return NSH_OK;
}
