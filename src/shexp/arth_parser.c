#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <stdlib.h>


static s_arth_ast *(*arth_parser_utils[])(char **start, char **end,
                                          s_env *env, s_errcont *cont) =
{
  ARTH_TYPE_APPLY(DECLARE_ARTH_PARSER_UTILS)
};


s_arth_ast *arth_parse_rec(char **start, char **end,
                           s_env *env, s_errcont *cont)
{
  if (/*ERROR || */start == end)
  {
    // TODO
    return NULL;
  }
  s_arth_ast *res = NULL;

  for (int i = 0; i < 13; i++)
    if ((res = arth_parser_utils[i](start, end, env, cont)))
      return res;
  // TODO
  return NULL;
}


void arth_free(s_arth_ast *ast)
{
  if (ast)
  {
    arth_free(ast->left);
    arth_free(ast->right);
    free(ast);
  }
}


s_arth_ast *arth_parse(char *str, s_env *env, s_errcont *cont)
{
  char **end;
  char **elms = arth_lex(str, &end);
  s_arth_ast *res = arth_parse_rec(elms, end, env, cont);
  free(elms);
  return res;
}
