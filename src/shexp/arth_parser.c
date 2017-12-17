#include "shexp/arth.h"
#include "utils/alloc.h"
#include "utils/strsplit.h"

#include <err.h>
#include <stdlib.h>


static void (*arth_parser_utils[])(char **start, char **end,
                                   s_arthcont *cont, s_arth_ast **ast) =
{
  ARTH_TYPE_APPLY(DECLARE_ARTH_PARSER_UTILS)
};


void arth_parse_rec(char **start, char **end,
                    s_arthcont *cont, s_arth_ast **ast)
{
  if (/*ERROR || */start == end)
  {
    // TODO
    warnx("syntax error: operand expected");
  }
  *ast = NULL;

  for (int i = 0; i < 13; i++)
  {
    arth_parser_utils[i](start, end, cont, ast);
    if (*ast)
      break;
  }
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


s_arth_ast *arth_parse(char *str, s_arthcont *cont)
{
  char **end;
  char **elms = arth_lex(str, &end);
  s_arth_ast *res = NULL;
  if (elms != end)
    arth_parse_rec(elms, end, cont, &res);
  free(elms);
  return res;
}
