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
s_ast *parse(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the list rule.
*/
s_ast *parse_list(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the and_or rule.
*/
s_ast *parse_and_or(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the pipeline rule.
*/
s_ast *parse_pipeline(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the command rule.
*/
s_ast *parse_command(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the simple_command rule.
*/
s_ast *parse_simple_command(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the shell_command rule.
*/
s_ast *parse_shell_command(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the funcdec rule.
*/
s_ast *parse_funcdec(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the redirection rule.
*/
s_ast *parse_redirection(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the compound_list rule.
*/
s_ast *parse_compound_list(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_for rule.
*/
s_ast *parse_rule_for(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_while rule.
*/
s_ast *parse_rule_while(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_until rule.
*/
s_ast *parse_rule_until(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_case rule.
*/
s_ast *parse_rule_case(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the rule_if rule.
*/
s_ast *parse_rule_if(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the else_clause rule.
*/
s_ast *parse_else_clause(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the do_group rule.
*/
s_ast *parse_do_group(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the case_clause rule.
*/
s_acase_node *parse_case_clause(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse the case_item rule.
*/
s_acase_node *parse_case_item(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse a word.
*/
s_wordlist *parse_word(s_lexer *lexer, s_errcont *errcont);

/**
** \brief parse every following newline.
*/
void parse_newlines(s_lexer *lexer, s_errcont *errcont);

/**
** \brief tell if next token starts a redirection.
*/
bool start_redir(const s_token *tok);
