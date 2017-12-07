#include "shlex/lexer.h"
#include "shlex/print.h"
#include "utils/macros.h"

#include <assert.h>

#define LEX_GEN_TOKS_STR_ENUM(Tokname) #Tokname,
#define LEX_CONST_STR_ENUM(TokName, Value) #TokName,


static const char *g_token_type_tab[] =
{
  LEX_OP_TOKS(LEX_CONST_STR_ENUM)
  LEX_KW_TOKS(LEX_CONST_STR_ENUM)
  LEX_GEN_TOKS(LEX_GEN_TOKS_STR_ENUM)
};


const char *token_type_to_string(enum token_type type)
{
  if (type > ARR_SIZE(g_token_type_tab))
    return NULL;
  return g_token_type_tab[type];
}


int print_tokens(FILE *f, s_cstream *cs, s_errcont *errcont)
{
  int res = 0;
  s_lexer *lex = lexer_create(cs);
  while (!cstream_eof(cs))
  {
    s_token *tok = lexer_pop(lex, errcont);
    if (!tok)
    {
      res = 1;
      break;
    }

    fprintf(f, "%zu:%zu\t%s(%s)[%c]\n", tok->lineinfo.line, tok->lineinfo.column,
           token_type_to_string(tok->type),
           TOK_STR(tok), tok->delim);
    tok_free(tok, true);
  }
  lexer_free(lex);
  return res;
}
