#include "shlex/lexer.h"
#include "utils/macros.h"

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
