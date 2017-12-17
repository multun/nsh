#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


s_arth_ast *arth_parse_pow(char **start, char **end,
                           s_arthcont *cont)
{
  const char *delim[] = { "**", NULL };
  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_POW, arth_parse_rec(start, pos, cont),
                  arth_parse_rec(pos + 1, end, cont));
  return ast;
}


s_arth_ast *arth_parse_time(char **start, char **end,
                            s_arthcont *cont)
{
  const char *delim[] = { "*", "/", NULL };
  char **pos = strsplit_r(start, end, delim, false);
  if (!pos)
    return NULL;

  int type = ARTH_TIME;
  if (**pos == '/')
    type = ARTH_DIV;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(type, arth_parse_rec(start, pos, cont),
                  arth_parse_rec(pos + 1, end, cont));
  return ast;
}


s_arth_ast *arth_parse_plus(char **start, char **end,
                            s_arthcont *cont)
{
  const char *delim[] = { "+", "-", NULL };
  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return NULL;

  int type = ARTH_PLUS;
  if (**pos == '-')
    type = ARTH_MINUS;

  free(*pos);
  *pos = NULL;

  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));

  if (pos == start)
    *ast = ARTH_AST(type, arth_parse_rec(start + 1, end, cont), NULL);
  else
    *ast = ARTH_AST(type, arth_parse_rec(start, pos, cont),
                    arth_parse_rec(pos + 1, end, cont));
  return ast;
}
