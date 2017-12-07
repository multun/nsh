#pragma once

/**
** \file shparse/parse.h
*/

#include "ast/ast.h"
#include "shlex/lexer.h"
#include "utils/error.h"

/**
** \brief parse the input of 42sh.
*/
void parse(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the list rule.
*/
void parse_list(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the and_or rule.
*/
void parse_and_or(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the pipeline rule.
*/
void parse_pipeline(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the command rule.
*/
void parse_command(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the simple_command rule.
*/
void parse_simple_command(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the shell_command rule.
*/
void parse_shell_command(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the funcdec rule.
*/
void parse_funcdec(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the redirection rule.
*/
void parse_redirection(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the compound_list rule.
*/
void parse_compound_list(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_for rule.
*/
void parse_rule_for(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_while rule.
*/
void parse_rule_while(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_until rule.
*/
void parse_rule_until(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_case rule.
*/
void parse_rule_case(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_if rule.
*/
void parse_rule_if(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the else_clause rule.
*/
void parse_else_clause(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the do_group rule.
*/
void parse_do_group(s_ast **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the case_clause rule.
*/
void parse_case_clause(s_acase_node **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the case_item rule.
*/
void parse_case_item(s_acase_node **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse a word.
*/
void parse_word(s_wordlist **res, s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse every following newline.
*/
void parse_newlines(s_lexer *lexer, s_errcont *errcont);

/**
** \brief tell if next token starts a redirection.
*/
bool start_redir(const s_token *tok);
