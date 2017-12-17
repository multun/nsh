#include "shexec/clean_exit.h"
#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <err.h>
#include <stdlib.h>


void arth_parse_not(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "!",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  if (pos != start)
  {
    warnx("syntax error in expression");
    clean_exit(cont->cont, 1);
  }

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_NOT, NULL, NULL);
  arth_parse_rec(start + 1, end, cont, &(*ast)->left);
}


void arth_parse_and(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "&&",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_AND, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}


void arth_parse_or(char **start, char **end,
                   s_arthcont *cont, s_arth_ast **ast)
{
  const char *delim[] =
  {
    "||",
    NULL,
  };

  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return;

  free(*pos);
  *pos = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_OR, NULL, NULL);
  arth_parse_rec(start, pos, cont, &(*ast)->left);
  arth_parse_rec(pos + 1, end, cont, &(*ast)->right);
}
