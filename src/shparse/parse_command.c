#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"


static bool is_first_keyword(const s_token *tok)
{
  return tok_is(tok, TOK_IF) || tok_is(tok, TOK_FOR)
         || tok_is(tok, TOK_WHILE) || tok_is(tok, TOK_UNTIL)
         || tok_is(tok, TOK_CASE);
}


static s_ast *redirection_loop_sec(s_lexer *lexer, s_errman *errman,
                                   s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  s_ast *redir = NULL;
  while (start_redir(tok))
  {// TODO: HEREDOC
    s_ast *tmp = parse_redirection(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    if (redir)
      redir->data.ast_redirection.action = tmp;
    else
      res->data.ast_block.redir = tmp;
    redir = tmp;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  return res;
}


static s_ast *redirection_loop(s_lexer *lexer, s_ast *cmd, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_BLOCK;
  res->data.ast_block = ABLOCK(NULL, NULL, cmd);
  res = redirection_loop_sec(lexer, errman, res);
  if (cmd->type == SHNODE_FUNCTION)
  {
    res->data.ast_block.cmd = cmd->data.ast_function.value;
    cmd->data.ast_function.value = res;
    return cmd;
  }
  return res;
}


static bool chose_shell_func(s_lexer *lexer, s_errman *errman, s_ast **res)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return true;
  bool shell = is_first_keyword(tok) || tok_is(tok, TOK_LBRACE)
               || tok_is(tok, TOK_LPAR);
  if (shell)
  {
    *res = parse_shell_command(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return true;
  }
  else if (tok_is(tok, TOK_FUNC))
  { // discard tokken 'function' and create token NAME to match latter use
    tok_free(lexer_pop(lexer, errman), true);
    *res = parse_funcdec(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return true;
  }
  else
    return false;
  return true;
}


s_ast *parse_command(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = NULL;
  if (!chose_shell_func(lexer, errman, &res))
  {
    s_token *word = lexer_pop(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    const s_token *tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    bool is_par = tok_is(tok, TOK_LPAR);
    lexer_push(lexer, word);
    if (is_par)
      res = parse_funcdec(lexer, errman);
    else
      return parse_simple_command(lexer, errman);
  }
  if (ERRMAN_FAILING(errman))
    return res;
  return redirection_loop(lexer, res, errman);
}


static s_ast *switch_first_keyword(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (tok_is(tok, TOK_IF))
    return parse_rule_if(lexer, errman);
  else if (tok_is(tok, TOK_FOR))
    return parse_rule_for(lexer, errman);
  else if (tok_is(tok, TOK_WHILE))
    return parse_rule_while(lexer, errman);
  else if (tok_is(tok, TOK_UNTIL))
    return parse_rule_until(lexer, errman);
  else if (tok_is(tok, TOK_CASE))
    return parse_rule_case(lexer, errman);
  sherror(&tok->lineinfo, errman, "unexpected token %s", TOKT_STR(tok));
  return NULL;
}


s_ast *parse_shell_command(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (tok_is(tok, TOK_LBRACE) || tok_is(tok, TOK_LPAR))
  {
    bool par = tok_is(tok, TOK_LPAR);
    tok_free(lexer_pop(lexer, errman), true);
    s_ast *res = parse_compound_list(lexer, errman);
    if (ERRMAN_FAILING(errman) || ((tok = lexer_peek(lexer, errman))
                                   && ERRMAN_FAILING(errman)))
      return res;
    if ((tok_is(tok, TOK_RBRACE) && !par)
        || (tok_is(tok, TOK_RPAR) && par))
    {
      tok_free(lexer_pop(lexer, errman), true);
      return res;
    }
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected '}' or ')'", TOKT_STR(tok));
    return res;
  }
  else
    return switch_first_keyword(lexer, errman);
}
