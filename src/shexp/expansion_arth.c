#include "utils/evect.h"
#include "shexp/expansion.h"
#include "shexp/arth.h"


#include <err.h>

char *s = { 0 };

void expand_arth(char **str, s_env *env, s_evect *vec)
{
  bool err = false;
  (void)env;
  (void)vec;
  s_arth_ast *ast = arth_parse(*str, &err);
  (void)ast;
  *str = s;
}
