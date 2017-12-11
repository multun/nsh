#include "shexp/arth.h"
#include "utils/strsplit.h"


void arth_default_index(int &index, char **elms)
{
  if (elms[*index])
  {
    free(elms[*index]);
    elms[*index] = NULL;
  }
  else
    *index = -1;
}


static s_arth_ast *arth_parse_and(char **elms, bool *err)
{
  char *delim = "&&";
  int i = strsplit_r(elms, &delim, 1);
  arth_default_index(&i, elms);
  s_arth_ast *res = arth_parse_bor(elms, err);
  while (i != -1 && !*err)
  {
    s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
    *ast = ARTH_AST(ARTH_AND, res, arth_parse_bor(elms, err));
    res = ast;
    i = strsplit_r(elms + i + 1, &delim, 1);
    arth_default_index(&i, elms);
  }
  return res;
}


static s_arth_ast *arth_parse_or(char **elms, bool *err)
{
  char *delim = "||";
  int i = strsplit_r(elms, &delim, 1);
  arth_default_index(&i, elms);
  s_arth_ast *res = arth_parse_and(elms, err);
  while (i != -1 && !*err)
  {
    s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
    *ast = ARTH_AST(ARTH_OR, res, arth_parse_and(elms, err));
    res = ast;
    i = strsplit_r(elms + i + 1, &delim, 1);
    arth_default_index(&i, elms);
  }
  return res;
}

s_arth_ast *arth_parse(char *str, bool *err)
{
  char **elms = arth_lex(str);
  s_arth_ast *res = arth_parse_or(elms, err);
  return res;
}
