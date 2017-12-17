#include "shexec/clean_exit.h"
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
    warnx("syntax error: operand expected");
    clean_exit(cont->cont, 1);
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
  if (!ast)
    return;

  arth_free(ast->left);
  arth_free(ast->right);
  free(ast);
}


s_arth_ast *arth_parse(char *str, s_arthcont *cont)
{
  char **end;
  char **elms = arth_lex(str, &end);
  s_arth_ast *res = NULL;
  if (elms != end)
  {
    s_keeper keeper = KEEPER(cont->cont->keeper);
    if (setjmp(keeper.env))
    {
      arth_free(res);
      if (elms)
        for (char **cur = elms; cur < end; cur++)
          free(*cur);
      free(elms);
      shraise(cont->cont, NULL);
    }
    else
    {
      s_arthcont ncnt = ARTHCONT(cont->env,
                                 &ERRCONT(cont->cont->errman, &keeper));
      arth_parse_rec(elms, end, &ncnt, &res);
    }
  }
  if (elms)
    for (char **cur = elms; cur < end; cur++)
      free(*cur);
  free(elms);
  return res;
}
