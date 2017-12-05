#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "shparse/parse.h"
#include "utils/alloc.h"


static s_ast *negate_ast(s_ast *ast, bool neg)
{
  if (!neg)
    return ast;
  s_ast *negation = xcalloc(sizeof(s_ast), 1);
  negation->type = SHNODE_BOOL_OP;
  negation->data.ast_bool_op = ABOOL_OP(BOOL_NOT, ast, NULL);
  return negation;
}


static s_ast *pipeline_loop(s_lexer *lexer, s_errman *errman, s_ast *res)
{
  const s_token *tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer, errman), true);
    parse_newlines(lexer, errman);
    tok = lexer_peek(lexer, errman);
    s_ast *pipe = xcalloc(sizeof(s_ast), 1);
    pipe->type = SHNODE_PIPE;
    pipe->data.ast_pipe = APIPE(res, parse_command(lexer, errman));
    res = pipe;
    if (ERRMAN_FAILING(errman))
      return res;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  return res;
}


s_ast *parse_pipeline(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  bool negation = tok_is(tok, TOK_BANG);
  if (negation)
    tok_free(lexer_pop(lexer, errman), true);

  s_ast *res = parse_command(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;

  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  res = pipeline_loop(lexer, errman, res);
  if (ERRMAN_FAILING(errman))
    return res;
  return negate_ast(res, negation);
}


enum redir_type parse_redir_type(const s_token *tok)
{
  if (tok_is(tok, TOK_LESS))
    return REDIR_LESS;
  if (tok_is(tok, TOK_DLESS))
    return REDIR_DLESS;
  if (tok_is(tok, TOK_GREAT))
    return REDIR_GREAT;
  if (tok_is(tok, TOK_DGREAT))
    return REDIR_DGREAT;
  if (tok_is(tok, TOK_LESSAND))
    return REDIR_LESSAND;
  if (tok_is(tok, TOK_GREATAND))
    return REDIR_GREATAND;
  if (tok_is(tok, TOK_LESSDASH))
    return REDIR_LESSDASH;
  if (tok_is(tok, TOK_LESSGREAT))
    return REDIR_LESSGREAT;
  if (tok_is(tok, TOK_CLOBBER))
    return REDIR_CLOBBER;
  abort();
}


s_ast *parse_redirection(s_lexer *lexer, s_errman *errman)
{
  s_token *tok = lexer_pop(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return NULL;
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_REDIRECTION;
  int left = -1;
  if (tok_is(tok, TOK_IO_NUMBER))
  {
    left = atoi(TOK_STR(tok));
    tok_free(tok, true);
    tok = lexer_pop(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return res;
  }
  enum redir_type type = parse_redir_type(tok);
  tok_free(tok, true);
  s_wordlist *word = parse_word(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  res->data.ast_redirection = AREDIRECTION(type, left, word, NULL);
  return res;
}
