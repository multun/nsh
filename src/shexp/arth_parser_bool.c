#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


s_arth_ast *arth_parse_not(char **start, char **end,
                           s_env *env, s_errcont *cont)
{
  const char *delim[] = { "!", NULL };
  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return NULL;

  if (pos != start)
  {
    // TODO
    return NULL;
  }

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_NOT, arth_parse_rec(start + 1, end, env, cont), NULL);
  return ast;
}


s_arth_ast *arth_parse_and(char **start, char **end,
                           s_env *env, s_errcont *cont)
{
  const char *delim[] = { "&&", NULL };
  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_AND, arth_parse_rec(start, pos, env, cont),
                  arth_parse_rec(pos + 1, end, env,cont));
  return ast;
}


s_arth_ast *arth_parse_or(char **start, char **end,
                          s_env *env, s_errcont *cont)
{
  const char *delim[] = { "||", NULL };
  char **pos = strsplit_r(start, end, delim, true);
  if (!pos)
    return NULL;

  free(*pos);
  *pos = NULL;
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_OR, arth_parse_rec(start, pos, env, cont),
                  arth_parse_rec(pos + 1, end, env, cont));
  return ast;
}
