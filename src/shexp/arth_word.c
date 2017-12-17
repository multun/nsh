#include "shexp/arth.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

#include <assert.h>
#include <err.h>
#include <stdlib.h>


static int wordtoi(char *str, bool *err)
{
  if (!(*str)) return 0;
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

static int parse_word(char *var, s_arthcont *cont)
{
  int max_rec = 15;

  while (max_rec && *var && (*var < '0' || *var > '9'))
  {
    char *newvar = expand_arth_word(var, cont->env, cont->cont);
    free(var);
    var = newvar;
    max_rec--;
  }

  bool err = false;
  int n = wordtoi(var, &err);

  if (!max_rec || err)
  {
    // TODO
    if (!max_rec)
      warnx("expression recursion level exceeded");
    else
      warnx("'%s': value to great for base", var);
    free(var);
    return 0;
  }
  free(var);
  return n;
}

void arth_parse_word(char **start, char **end,
                     s_arthcont *cont, s_arth_ast **ast)
{
  assert(start < end);
  int n = parse_word(*start, cont);
  *start = NULL;
  *ast = xcalloc(1, sizeof(s_arth_ast));
  **ast = ARTH_AST(ARTH_WORD, NULL, NULL);
  (*ast)->value = n;
}
