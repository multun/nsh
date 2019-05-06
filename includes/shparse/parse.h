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
void parse(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the list rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_list(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the and_or rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_and_or(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the pipeline rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_pipeline(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_command(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the simple_command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_simple_command(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the shell_command rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_shell_command(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the funcdec rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_funcdec(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the redirection rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_redirection(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the compound_list rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_compound_list(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_for rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_for(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_while rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_while(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_until rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_until(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_case rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_case(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_if rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_rule_if(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the else_clause rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_else_clause(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the do_group rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_do_group(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the case_clause rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_case_clause(s_acase_node **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the case_item rule.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_case_item(s_acase_node **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse a word.
**
** \param res buffer to store the parsing results.
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_word(s_wordlist **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse every following newline.
**
** \param lexer lexer to use in parsing.
** \param errcont error context.
*/
void parse_newlines(s_lexer *lexer, s_errcont *errcont);

/**
** \brief tell if given token starts a redirection.
**
** \param tok token that may start a redirection.
*/
bool start_redir(const s_token *tok);
