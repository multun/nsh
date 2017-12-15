#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>



static s_arth_ast *arth_parse_or(char **start, char **end, bool *err)
{
  const char *delim = "||";
  int i = strsplit_r(start, &delim, 1);
  if (i == end - start)
    return NULL;

  free(start[i]);
  start[i] = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_OR, arth_parse_rec(start, start + i, err),
                  arth_parse_rec(start + i + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_rec(char **start, char **end, bool *err)
{
  if (*err || start == end)
  {
    *err = true;
    return NULL;
  }
  s_arth_ast *res = NULL;
  if ((res = arth_parse_or(start, end, err)))
    return res;
  return arth_parse_word(start, err);
}

s_arth_ast *arth_parse(char *str, bool *err)
{
  char **end;
  char **elms = arth_lex(str, &end);
  s_arth_ast *res = arth_parse_or(elms, end, err);
  return res;
}
