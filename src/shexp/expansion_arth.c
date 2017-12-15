#include "utils/evect.h"
#include "shexp/expansion.h"
#include "shexp/arth.h"


#include <err.h>
#include <string.h>


void expand_arth(char **str, s_env *env, s_evect *vec)
{
  bool err = false;
  (void)env;
  (*str)++;
  char *tmp = strrchr(*str, ')');
  *tmp = 0;
  tmp = strrchr(*str, ')');
  *tmp = 0;
  s_arth_ast *ast = arth_parse(*str, &err);
  *str = tmp + 1;
  char tab[12];
  int res = arth_exec(ast);
  sprintf(tab, "%d", res);
  for (char *c = tab; *c; c++)
    evect_push(vec, *c);
}
