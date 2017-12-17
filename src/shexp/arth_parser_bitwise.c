#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <err.h>
#include <stdlib.h>


void arth_parse_bnot(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] = {
    "~",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  if (pos != start)
  {
    // TODO
    warnx("syntax error in expression");
  }

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_BNOT, NULL, NULL);
  arth_parse_rec(start + 1, end, cont, &(*ast)->left);
}


void arth_parse_band(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "&",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_BAND, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}


void arth_parse_xor(char **start, char **end,
                   s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "^",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_XOR, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}


void arth_parse_bor(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "|",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return ;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_BOR, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}
