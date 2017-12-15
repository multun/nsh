#include "shexp/arth.h"
#include "utils/alloc.h"

#include <err.h>


static int wordtoi(char *str, bool *err)
{
  if (!*str)
    return 0;
  int n = 0;
  bool neg = str[0] == '-';
  str += neg;
  for (; *str; str++)
  {
    if (*str < '0' || *str > '9')
      *err = true;
    n = n * 10 + *str - '0';
  }
  return neg ? -n : n;
}

s_arth_ast *arth_parse_word(char **start, bool *err)
{
  int n = wordtoi(*start, err);
  if (*err)
  {
    warnx("%s: value is NaN", *start);
    return NULL;
  }
  s_arth_ast *ast = xcalloc(1, sizeof(s_arth_ast));
  *ast = ARTH_AST(ARTH_WORD, NULL, NULL);
  ast->value = n;
  return ast;
}
