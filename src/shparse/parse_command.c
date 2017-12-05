#include <string.h>
#include <assert.h>

#include "shparse/parse.h"
#include "shlex/print.h"
#include "utils/alloc.h"
#include "utils/error.h"


static bool start_redir(const s_token *tok)
{
  return tok_is(tok, TOK_IO_NUMBER) || tok_is(tok, TOK_DLESS)
         || tok_is(tok, TOK_DGREAT) || tok_is(tok, TOK_LESSAND)
         || tok_is(tok, TOK_GREATAND) || tok_is(tok, TOK_LESSGREAT)
         || tok_is(tok, TOK_LESSDASH) || tok_is(tok, TOK_CLOBBER)
         || tok_is(tok, TOK_LESS) || tok_is(tok, TOK_GREAT);
}


static bool is_first_keyword(const s_token *tok)
{
  return tok_is(tok, TOK_IF) || tok_is(tok, TOK_FOR)
         || tok_is(tok, TOK_WHILE) || tok_is(tok, TOK_UNTIL)
         || tok_is(tok, TOK_CASE);
}


static s_ast *redirection_loop(s_lexer *lexer, s_ast *cmd, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_BLOCK;
  res->data.ast_block = ABLOCK(NULL, NULL, cmd);
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
  if (cmd->type == SHNODE_FUNCTION)
  {
    res->data.ast_block.cmd = cmd->data.ast_function.value;
    cmd->data.ast_function.value = res;
    return cmd;
  }
  return res;
}


s_ast *parse_command(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  bool shell = is_first_keyword(tok) || tok_is(tok, TOK_LBRACE)
               || tok_is(tok, TOK_LPAR);
  s_ast *res = NULL;
  if (shell)
  {
    res = parse_shell_command(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  else if (tok_is(tok, TOK_FUNC))
  { // discard tokken 'function' and create token NAME to match latter use
    tok_free(lexer_pop(lexer, errman), true);
    res = parse_funcdec(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  else
  {
    s_token *word = lexer_pop(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
    tok = lexer_peek(lexer, errman);
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


static s_ast *parse_assignment(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_ASSIGNMENT;
  s_token *tok = lexer_pop(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  char * val = strchr(TOK_STR(tok), '=');
  *val = '\0';
  val++;
  s_wordlist *name = xcalloc(sizeof(s_wordlist), 1);
  *name = WORDLIST(TOK_STR(tok), false, false, NULL);
  s_wordlist *value = xcalloc(sizeof(s_wordlist), 1);
  *value = WORDLIST(val, true, true, NULL);
  tok_free(tok, false);
  res->data.ast_assignment = AASSIGNMENT(name, value, NULL);
  return res;
}


static bool prefix_loop(s_lexer *lexer, s_ablock *block, s_ast **redirect,
                       s_errman *errman)
{
  s_ast *assign = NULL;
  s_ast *redir = NULL;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  while (tok_is(tok, TOK_ASSIGNMENT_WORD) || start_redir(tok))
  {
    s_ast *tmp = NULL;
    if (tok_is(tok, TOK_ASSIGNMENT_WORD))
    {
      tmp = parse_assignment(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return false;
      if (assign)
        assign->data.ast_assignment.action = tmp;
      else
        block->def = tmp;
      assign = tmp;
    }
    else
    { // TODO: HEREDOC
      tmp = parse_redirection(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return false;
      if (redir)
        redir->data.ast_redirection.action = tmp;
      else
        block->redir = tmp;
      redir = tmp;
    }
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
  }
  *redirect = redir;
  return true;
}


static bool element_loop(s_lexer *lexer, s_ablock *block, s_ast *redir,
                        s_errman *errman)
{
  s_wordlist *elm = NULL;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  while (tok_is(tok, TOK_WORD) || start_redir(tok))
  {
    if (tok_is(tok, TOK_WORD))
    {
      s_wordlist *tmp = parse_word(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return false;
      if (elm)
        elm->next = tmp;
      else
      {
        block->cmd = xcalloc(sizeof(s_ast), 1);
        block->cmd->type = SHNODE_CMD;
        block->cmd->data.ast_cmd = ACMD(tmp);
      }
      elm = tmp;
    }
    else
    { // TODO: HEREDOC
      s_ast *tmp = NULL;
      tmp = parse_redirection(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return false;
      if (redir)
        redir->data.ast_redirection.action = tmp;
      else
        block->redir = tmp;
      redir = tmp;
    }
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
  }
  return true;
}


s_ast *parse_simple_command(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_BLOCK;
  res->data.ast_block = ABLOCK(NULL, NULL, NULL);
  s_ast *redir = NULL;
  if (!prefix_loop(lexer, &res->data.ast_block, &redir, errman)
      || !element_loop(lexer, &res->data.ast_block, redir, errman))
    return res;
  if (!res->data.ast_block.redir && !res->data.ast_block.def
      && !res->data.ast_block.cmd)
  {
    const s_token *tok = lexer_peek(lexer, errman);
    assert(!ERRMAN_FAILING(errman));
    sherror(&tok->lineinfo, errman, "parsing error %s", TOKT_STR(tok));
    return res;
  }
  return res;
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
    if (ERRMAN_FAILING(errman))
      return res;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
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


s_ast *parse_funcdec(s_lexer *lexer, s_errman *errman)
{
  s_token *word = lexer_pop(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (!tok_is(tok, TOK_LPAR))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected '('", TOKT_STR(tok));
    return NULL;
  }
  tok_free(lexer_pop(lexer, errman), true);
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  if (!tok_is(tok, TOK_RPAR))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected '('", TOKT_STR(tok));
    return NULL;
  }
  tok_free(lexer_pop(lexer, errman), true);
  parse_newlines(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_FUNCTION;
  s_wordlist *name = xcalloc(sizeof(word), 1);
  *name = WORDLIST(TOK_STR(word), true, true, NULL);
  res->data.ast_function = AFUNCTION(name, parse_shell_command(lexer, errman));
  return res;
}
