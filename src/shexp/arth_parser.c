#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


static s_arth_ast *arth_parse_and(char **start, char **end, bool *err)
{
  const char *delim = "&&";
  int i = strsplit_r(start, &delim, 1);
  if (i == end - start)
    return NULL;

  free(start[i]);
  start[i] = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_AND, arth_parse_rec(start, start + i, err),
                  arth_parse_rec(start + i + 1, end, err));
  return ast;
}


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


static s_arth_ast *(*arth_parser_utils[])(char **start, char **end, bool *err) =
{
  ARTH_TYPE_APPLY(DECLARE_ARTH_PARSER_UTILS)
};


s_arth_ast *arth_parse_rec(char **start, char **end, bool *err)
{
  if (*err || start == end)
  {
    *err = true;
    return NULL;
  }
  s_arth_ast *res = NULL;

  for (int i = 0; i < 13; i++)
    if ((res = arth_parser_utils[i](start, end, err)))
      return res;
  *err = true;
  return NULL;
}


s_arth_ast *arth_parse(char *str, bool *err)
{
  char **end;
  char **elms = arth_lex(str, &end);
  s_arth_ast *res = arth_parse_rec(elms, end, err);
  return res;
}
