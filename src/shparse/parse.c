#include "shparse/parse.h"
#include "utils/alloc.h"

s_ast *parse(s_lexer *lexer)
{
  const s_token *tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return NULL;
  s_ast *res = parse_list(lexer);
  // TODO: handle parsing error
  tok = lexer_peek(lexer);
  if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
    return res;
  // TODO: parsing failed unxepected token exected EOF or '\n'
  return NULL;
}

s_ast *parse_list(s_lexer *lexer)
{
  s_ast *res = xmalloc(sizeof(s_ast));
  res->data.ast_list = ALIST(parse_and_or(lexer), NULL);
  const s_token *tok = lexer_peek(lexer);
  s_alist *tmp = &res->data.ast_list;
  while (tok_is(tok, TOK_SEMI) || tok_is(tok, TOK_AND))
  {
    s_token *tmp_tok = lexer_pop(lexer);
    tok_free(tmp_tok);
    tok = lexer_peek(lexer);
    if (tok_is(tok, TOK_EOF) || tok_is(tok, TOK_NEWLINE))
      return res;
    s_alist *next = xmalloc(sizeof(s_alist));
    *next = ALIST(parse_and_or(lexer), NULL);
    // TODO: handle parsing error
    tmp->next = next;
    tmp = next;
  }
  return res;
}
