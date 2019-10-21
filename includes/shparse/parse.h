#pragma once

/**
** \file shparse/parse.h
*/

#include "ast/ast.h"
#include "shlex/lexer.h"
#include "shparse/parser_error.h"
#include "utils/error.h"

#define PARSER_ERROR(LineInfo, Errcont, ...)                                             \
    sherror((LineInfo), (Errcont), &g_lexer_error, __VA_ARGS__)

/**
** \brief parse the input of 42sh.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the list rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_list(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the and_or rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_and_or(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the pipeline rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_pipeline(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_command(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the simple_command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_simple_command(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the shell_command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_shell_command(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the funcdec rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_funcdec(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the redirection rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_redirection(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the compound_list rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_compound_list(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_for rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_for(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_while rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_while(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_until rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_until(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_case rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_case(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the rule_if rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_if(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the else_clause rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_else_clause(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the do_group rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_do_group(struct ast **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the case_clause rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_case_clause(struct acase_node **res, struct lexer *lexer, struct errcont *errcont);

/**
** \brief parse the case_item rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_case_item(struct acase_node **res, struct lexer *lexer, struct errcont *errcont);

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

/**
** \brief tell if given token starts a redirection.
**
** \param tok token that may start a redirection.
*/
bool start_redir(const struct token *tok);
