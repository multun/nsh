#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


s_arth_ast *arth_parse_pow(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "**", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_POW, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_div(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "/", false);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_DIV, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_time(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "*", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_TIME, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_minus(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "-", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));

  if (pos == start)
    *ast = ARTH_AST(ARTH_MINUS, arth_parse_rec(start + 1, end, err), NULL);
  else
    *ast = ARTH_AST(ARTH_MINUS, arth_parse_rec(start, pos, err),
                    arth_parse_rec(pos + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_plus(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "+", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;

  if (pos == start)
    return arth_parse_rec(start + 1, end, err);

  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_PLUS, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}
