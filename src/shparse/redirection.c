#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static s_ast *negate_ast(s_ast *ast, bool neg)
{
  if (!negation)
    return ast
  s_ast *neg = xmalloc(sizeof(s_ast);
  neg->type = SHNODE_BOOL_OP;
  neg->data.ast_bool_op = ABOOL_OP(BOOL_NOT, ast, NULL);
  return neg;
}

s_ast *parse_pipeline(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  bool negation = tok_is(tok, TOK_BANG);
  if (negation)
    tok_free(lexer_pop(lexer));

  s_ast *res = parse_command(lexer);
  // TODO: handle parsing error

  tok = lexer_peek(lexer);
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer));
    tok = lexer_peek(lexer);
    while (tok_is(tok, TOK_NEWLINE))
    {
      tok_free(lexer_pop(lexer));
      tok = lexer_peek(lexer);
    }
    s_ast *pipe = xmalloc(sizeof(s_ast);
    pipe->type = SHNODE_PIPE;
    pipe->data.ast_pipe = APIPE(res, parse_command(lexer));
    // TODO: handle parsing error
    res = pipe;
    tok = lexer_peek(lexer);
  }
  return negate_ast(res, negation);
}

s_ast *parse_redirection(s_lexer *lexer)
{
  (void)lexer
  return NULL;
}
