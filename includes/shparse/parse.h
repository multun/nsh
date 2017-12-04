#pragma once

#include "ast/ast.h"
#include "shlex/lexer.h"
#include "utils/error.h"

s_ast *parse(s_lexer *lexer, s_errman *errman);

s_ast *parse_list(s_lexer *lexer, s_errman *errman);
s_ast *parse_and_or(s_lexer *lexer, s_errman *errman);
s_ast *parse_pipeline(s_lexer *lexer, s_errman *errman);
s_ast *parse_command(s_lexer *lexer, s_errman *errman);
s_ast *parse_simple_command(s_lexer *lexer, s_errman *errman);
s_ast *parse_shell_command(s_lexer *lexer, s_errman *errman);
s_ast *parse_funcdec(s_lexer *lexer, s_errman *errman);
s_ast *parse_redirection(s_lexer *lexer, s_errman *errman);
s_ast *parse_compound_list(s_lexer *lexer, s_errman *errman);

s_ast *parse_rule_for(s_lexer *lexer, s_errman *errman);
s_ast *parse_rule_while(s_lexer *lexer, s_errman *errman);
s_ast *parse_rule_until(s_lexer *lexer, s_errman *errman);
s_ast *parse_rule_case(s_lexer *lexer, s_errman *errman);
s_ast *parse_rule_if(s_lexer *lexer, s_errman *errman);

s_ast *parse_else_clause(s_lexer *lexer, s_errman *errman);
s_ast *parse_do_group(s_lexer *lexer, s_errman *errman);
s_acase_node *parse_case_clause(s_lexer *lexer, s_errman *errman);
s_acase_node *parse_case_item(s_lexer *lexer, s_errman *errman);

s_wordlist *parse_word(s_lexer *lexer, s_errman *errman);
void parse_newlines(s_lexer *lexer, s_errman *errman);
