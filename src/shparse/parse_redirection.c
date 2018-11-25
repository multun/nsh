#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "shparse/parse.h"
#include "utils/alloc.h"


// this operation is atomic, no need to pass the target as argument
static void negate_ast(s_ast **ast, bool neg)
{
  if (!neg)
    return;

  s_ast *negation = xcalloc(sizeof(s_ast), 1);
  negation->type = SHNODE_BOOL_OP;
  negation->data.ast_bool_op = ABOOL_OP(BOOL_NOT, *ast, NULL);
  *ast = negation;
}



static void pipeline_loop(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  while (tok_is(tok, TOK_PIPE))
  {
    tok_free(lexer_pop(lexer, errcont), true);
    parse_newlines(lexer, errcont);
    tok = lexer_peek(lexer, errcont);
    s_ast *pipe = xcalloc(sizeof(s_ast), 1);
    pipe->type = SHNODE_PIPE;
    pipe->data.ast_pipe = APIPE(*res, NULL);
    *res = pipe;
    parse_command(&pipe->data.ast_pipe.right, lexer, errcont);
    tok = lexer_peek(lexer, errcont);
  }
}


void parse_pipeline(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  bool negation = tok_is(tok, TOK_BANG);
  if (negation)
    tok_free(lexer_pop(lexer, errcont), true);

  parse_command(res, lexer, errcont);
  tok = lexer_peek(lexer, errcont);
  pipeline_loop(res, lexer, errcont);
  negate_ast(res, negation);
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
  abort(); // TODO: raise exception
}


void parse_redirection(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  s_token *tok = lexer_pop(lexer, errcont);

  // TODO: alloc later to avoid potential leak on exception
  *res = xcalloc(sizeof(s_ast), 1);
  (*res)->type = SHNODE_REDIRECTION;
  int left = -1;
  if (tok_is(tok, TOK_IO_NUMBER))
  {
    left = atoi(tok_buf(tok));
    tok_free(tok, true);
    tok = lexer_pop(lexer, errcont);
  }

  enum redir_type type = parse_redir_type(tok);
  tok_free(tok, true);
  (*res)->data.ast_redirection = AREDIRECTION(type, left, NULL, NULL);
  parse_word(&(*res)->data.ast_redirection.right, lexer, errcont);
}
