#include "shparse/parse.h"
#include "utils/alloc.h"
#include "shlex/print.h"


static bool parse_func_remove_par(s_lexer *lexer, s_errman *errman)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  if (!tok_is(tok, TOK_LPAR))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected '('", TOKT_STR(tok));
    return false;
  }
  tok_free(lexer_pop(lexer, errman), true);
  tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  if (!tok_is(tok, TOK_RPAR))
  {
    sherror(&tok->lineinfo, errman,
            "unexpected token %s, expected '('", TOKT_STR(tok));
    return false;
  }
  return true;
}

s_ast *parse_funcdec(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_FUNCTION;

  s_token *word = lexer_pop(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  s_wordlist *name = xcalloc(sizeof(s_wordlist), 1);
  *name = WORDLIST(TOK_STR(word), true, true, NULL);
  res->data.ast_function.name = name;
  tok_free(word, false);
  if (!parse_func_remove_par(lexer, errman))
    return res;

  tok_free(lexer_pop(lexer, errman), true);
  parse_newlines(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  res->data.ast_function.value = parse_shell_command(lexer, errman);
  return res;
}
