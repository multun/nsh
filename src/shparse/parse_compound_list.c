#include <stdbool.h>

#include "shparse/parse.h"
#include "utils/alloc.h"

static bool compound_list_end(const s_token *tok)
{
  return tok_is(tok, TOK_THEN) || tok_is(tok, TOK_ELSE)
         || tok_is(tok, TOK_FI) || tok_is(tok, TOK_DONE)
         || tok_is(tok, TOK_ESAC) || tok_is(tok, TOK_ELIF)
         || tok_is(tok, TOK_DSEMI) || tok_is(tok, TOK_DO);
}

static bool compound_list_loop(s_lexer *lexer, s_alist **tail, s_errman *errman)
{
  parse_newlines(lexer, errman);
  const s_token *tok = lexer_peek(lexer, errman);
  if (compound_list_end(tok))
    return true;
  (*tail)->next = xmalloc(sizeof(s_alist));
  *(*tail)->next = ALIST(parse_and_or(lexer, errman), NULL);
  // TODO: handle parsing error
  *tail = (*tail)->next;
  return false;
}

s_ast *parse_compound_list(s_lexer *lexer, s_errman *errman)
{
  parse_newlines(lexer, errman);
  s_ast *res = xmalloc(sizeof(s_ast));
  res->type = SHNODE_LIST;
  res->data.ast_list = ALIST(parse_and_or(lexer, errman), NULL);
  // TODO: handle parsing error
  s_alist *tmp = &res->data.ast_list;
  const s_token *tok = lexer_peek(lexer, errman);
  while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND)
         || tok_is(tok, TOK_NEWLINE))
  { // TODO: & = background task
    tok_free(lexer_pop(lexer, errman), true);
    if (compound_list_loop(lexer, &tmp, errman))
      break;
  }
  return res;
}
