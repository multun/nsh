#pragma once

/**
** \file shparse/parse.h
*/

#include "shparse/ast.h"
#include "shlex/lexer.h"
#include "shlex/print.h"
#include "shparse/parser_error.h"
#include "utils/error.h"

#include <stdarg.h>

__noreturn static inline void parser_err(const struct lineinfo *lineinfo, struct errcont *errcont, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsherror(lineinfo, errcont, &g_parser_error, fmt, ap);

    va_end(ap);
}

static inline void parser_consume(struct lexer *lexer, enum token_type type, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (!tok_is(tok, type))
        parser_err(&tok->lineinfo, errcont, "unexpected token %s, expected %s",
                     tok_buf(tok), token_type_to_string(type));

    lexer_discard(lexer, errcont);
}

static inline void parser_consume_optional(struct lexer *lexer, enum token_type type, struct errcont *errcont)
{
    const struct token *tok = lexer_peek(lexer, errcont);
    if (tok_is(tok, type))
        lexer_discard(lexer, errcont);
}

/**
** \brief parse the input of 42sh.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the list rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_list(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the and_or rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_and_or(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the pipeline rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_pipeline(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_command(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the simple_command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_simple_command(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the funcdec rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_funcdec(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the redirection rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
int parse_redirection(struct redir_vect *res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the compound_list rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_compound_list(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_for rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_for(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_while rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_while(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_until rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_until(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_case rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_case(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_if rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_if(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the else_clause rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_else_clause(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the do_group rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_do_group(struct shast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse a word.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
char *parse_word(struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse every following newline.
**
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_newlines(struct lexer *lexer, struct errcont *errcont);
