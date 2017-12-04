#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static s_ast *negate_ast(s_ast *ast, bool neg)
{
  if (!neg)
    return ast;
  s_ast *negation = xmalloc(sizeof(s_ast));
  negation->type = SHNODE_BOOL_OP;
  negation->data.ast_bool_op = ABOOL_OP(BOOL_NOT, ast, NULL);
  return negation;
}

s_ast *parse_pipeline(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  bool negation = tok_is(tok, TOK_BANG);
  if (negation)
    tok_free(lexer_pop(lexer, errman), true);

  s_ast *res = parse_command(lexer, errman);
  // TODO: handle parsing error

  tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer, errman), true);
    parse_newlines(lexer, errman);
    tok = lexer_peek(lexer, errman);
    s_ast *pipe = xmalloc(sizeof(s_ast));
    pipe->type = SHNODE_PIPE;
    pipe->data.ast_pipe = APIPE(res, parse_command(lexer, errman));
    // TODO: handle parsing error
    res = pipe;
    tok = lexer_peek(lexer, errman);
  }
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
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_REDIRECTION;
  int left = -1;
  if (tok_is(tok, TOK_IO_NUMBER))
  {
    left = atoi(TOK_STR(tok));
    tok_free(tok, true);
    tok = lexer_pop(lexer, errman);
  }
  enum redir_type type = parse_redir_type(tok);
  tok_free(tok, true);
  s_wordlist *word = parse_word(lexer, errman);
  res->data.ast_redirection = AREDIRECTION(type, left, word, NULL);
  return res;
}
