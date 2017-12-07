#include "shparse/parse.h"
#include "utils/alloc.h"
#include "shlex/print.h"


static bool parse_func_remove_par(s_lexer *lexer, s_errcont *errcont)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  if (!tok_is(tok, TOK_LPAR))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected '('", TOKT_STR(tok));
  tok_free(lexer_pop(lexer, errcont), true);
  tok = lexer_peek(lexer, errcont);
  if (!tok_is(tok, TOK_RPAR))
    sherror(&tok->lineinfo, errcont,
            "unexpected token %s, expected '('", TOKT_STR(tok));
  return true;
}

void parse_funcdec(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  *res = xcalloc(sizeof(s_ast), 1);
  (*res)->type = SHNODE_FUNCTION;

  s_token *word = lexer_pop(lexer, errcont);
  s_wordlist *name = xcalloc(sizeof(s_wordlist), 1);
  *name = WORDLIST(TOK_STR(word), true, true, NULL);
  (*res)->data.ast_function.name = name;
  tok_free(word, false);
  if (!parse_func_remove_par(lexer, errcont))
    return;

  tok_free(lexer_pop(lexer, errcont), true);
  parse_newlines(lexer, errcont);
  parse_shell_command(&(*res)->data.ast_function.value, lexer, errcont);
}
