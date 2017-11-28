#include <string.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static bool is_first_keyword(const s_token *tok)
{
  return tok_is(tok, TOK_IF) || tok_is(tok, TOK_FOR)
         || tok_is(tok, TOK_WHILE) || tok_is(tok, TOK_UNTIL)
         || tok_is(tok, TOK_CASE);
}

s_ast *parse_command(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  bool shell = is_first_keyword(tok) || tok_is(tok, TOK_LBRACE)
               || tok_is(tok, TOK_LPAR);
  s_ast *res = NULL;
  if (shell)
  {
    //tok_free(lexer_pop(lexer));
    res = parse_shell_command(lexer);
  }
  else if (tok_is(tok, TOK_FUNC))
  { // discard tokken 'function' and create token NAME to match latter use
    //tok_free(lexer_pop(lexer));
    res = parse_funcdec(lexer, lexer_pop(lexer));
  }  
  else 
  {
    s_token *word = lexer_pop(lexer);
    if (tok_is(lexer_peek(lexer), TOK_LPAR))
      res = parse_funcdec(lexer, word);
    else
      return parse_simple_command(lexer, word);
  }
  // parse redirection*
  return res;
}

static bool start_redir(const s_token *tok)
{
  (void)tok;
  return false;
}

#define P_ASSIGNEMENT(name, value)            \
{                                             \
  .name = WORDLIST(name, true, true, NULL),   \
  .value = WORDLIST(value, true, true, NULL), \
  .action = NULL,                             \
}

s_ast *parse_simple_command(s_lexer *lexer, s_token *word)
{
  const s_token *tok = word;
  s_ast *redir_head = NULL;
  s_ast *assign_head = NULL;
  s_ast *elm_head = NULL;
  s_ast *redir = NULL;
  s_ast *assign = NULL;
  s_wordlist *elm = NULL;
  while (tok_is(tok, TOK_ASSIGNEMENT_WORD) || start_redir(tok))
  {
    s_ast *tmp = NULL;
    if (tok_is(tok, TOK_ASSIGNEMENT_WORD))
    {
      tmp = parse_prefix(lexer);
      if (assign)
        assign->data.ast_assignement.action = tmp;
      else
        assign_head = tmp;
      assign = tmp;
    }
    else
    { // TODO: HEREDOC
      tmp = parse_redirection(lexer);
      if (redir)
        redir->data.ast_redirection.action = tmp;
      else
        redir_head = tmp;
      redir = tmp;
    }
    tok = lexer_peek(lexer);
  }
  while (tok_is(tok, TOK_WORD) || start_redir(tok))
  {
    if (tok_is(tok, TOK_WORD))
    {
      s_wordlist *tmp = parse_word(lexer);
      if (elm)
        elm->next = tmp;
      else
      {
        elm_head = xmalloc(sizeof(s_ast));
        elm_head->type = SHNODE_CMD;
        elm_head->data.ast_cmd = ACMD(tmp);
      }
      elm = tmp;
    }
    else
    { // TODO: HEREDOC
      s_ast *tmp = NULL;
      tmp = parse_redirection(lexer);
      if (redir)
        redir->data.ast_redirection.action = tmp;
      else
        redir_head = tmp;
      redir = tmp;
    }
    tok = lexer_peek(lexer);
  }
  if (!redir_head && !elm_head && !assign_head)
  { // TODO: handle parsing error
    return NULL;
  }
  s_ast *res = elm_head;
  if (assign_head)
  {
    assign->data.ast_assignement.action = res;
    res = assign_head;
  }
  if (redir_head)
  {
    redir->data.ast_redirection.action = res;
    res = redir_head;
  }
  return res;
}

static s_ast *switch_first_keyword(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_IF))
    return parse_rule_if(lexer);
  else if (tok_is(tok, TOK_FOR))
    return parse_rule_for(lexer);
  else if (tok_is(tok, TOK_WHILE))
    return parse_rule_while(lexer);
  else if (tok_is(tok, TOK_UNTIL))
    return parse_rule_until(lexer);
  else if (tok_is(tok, TOK_CASE))
    return parse_rule_case(lexer);
  // TODO: handle parsing error
  return NULL;
}

s_ast *parse_shell_command(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_LBRACE) || tok_is(tok, TOK_LPAR))
  {
    bool par = tok_is(tok, TOK_LPAR);
    //tok_free(lexer_pop(lexer));
    s_ast *res = parse_compound_list(lexer);
    tok = lexer_peek(lexer);
    if ((tok_is(tok, TOK_LBRACE) && !par)
        || (tok_is(tok, TOK_LPAR) && par))
    {
      //tok_free(lexer_pop(lexer));
      return res;
    }
    return NULL;
  }
  else
  {
    s_ast *res = switch_first_keyword(lexer);
    return res;
  }
}

s_ast *parse_funcdec(s_lexer *lexer, s_token *word)
{
  //tok_free(lexer_pop(lexer)); // first '(' was checked in commande
  if (!tok_is(lexer_peek(lexer), TOK_RPAR))
  { // TODO: handle parsing error
    return NULL;
  }
  //tok_free(lexer_pop(lexer));
  const s_token *tok = lexer_peek(lexer);
  while (tok_is(tok, TOK_NEWLINE))
  {
    //tok_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
  }
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_FUNCTION;
  s_wordlist *name = xmalloc(sizeof(word));
  *name = WORDLIST(TOK_STR(word), true, true, NULL);
  res->data.ast_function = AFUNCTION(name, parse_shell_command(lexer));
  // TODO: handle parsing error
  return res;
}
