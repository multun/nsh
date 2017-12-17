#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


s_arth_ast *arth_parse_bnot(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "~", true);
  if (!pos)
    return NULL;

  if (pos != start)
  {
    *err = true;
    return NULL;
  }

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_BNOT, arth_parse_rec(start + 1, end, err), NULL);
  return ast;
}


s_arth_ast *arth_parse_band(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "&", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_BAND, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_xor(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "^", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_XOR, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}


s_arth_ast *arth_parse_bor(char **start, char **end, bool *err)
{
  char **pos = strsplit_r(start, end, "|", true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_BOR, arth_parse_rec(start, pos, err),
                  arth_parse_rec(pos + 1, end, err));
  return ast;
}
