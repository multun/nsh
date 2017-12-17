#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


void arth_parse_pow(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "**",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_POW, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}


void arth_parse_time(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "*",
    "/",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, false);
  if (!pos)
    return;

  int type = ARTH_TIME;
  if (**pos == '/')
    type = ARTH_DIV;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(type, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}


void arth_parse_plus(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "+",
    "-",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  *ast = xcalloc(1, sizeof(s_arth_ast));

  if (**pos == '-')
    **ast = ARTH_AST(ARTH_MINUS, NULL, NULL);
  else
    **ast = ARTH_AST(ARTH_PLUS, NULL, NULL);

  free(*pos);
  *pos = NULL;


  if (pos == start)
    arth_parse_rec(start + 1, end, cont, &(*ast)->left);
  else
  {
    arth_parse_rec(start, pos, cont, &(*ast)->left);
    arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
  }
}
