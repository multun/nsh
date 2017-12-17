#include "utils/evect.h"
#include "shexp/expansion.h"
#include "shexp/arth.h"


#include <err.h>
#include <string.h>


char *expand_arth_word(char *word, s_env *env, s_errcont *cont)
{
  s_evect vec;
  evect_init(&vec, strlen(word) + 3);
  bool ndollar = *word != '$';
  if (ndollar)
  {
    evect_push(&vec, '$');
    evect_push(&vec, '{');
  }
  for (; *word; word++)
    evect_push(&vec, *word);
  if (ndollar)
    evect_push(&vec, '}');
  evect_push(&vec, '\0');

  char *res = expand(vec.data, env, cont);
  evect_destroy(&vec);
  return res;
}


void expand_arth(char **str, s_env *env, s_evect *vec, s_errcont *cont)
{
  (*str)++;
  char *tmp = strrchr(*str, ')');
  if (!tmp)
    return;
  *tmp = 0;
  tmp = strrchr(*str, ')');
  if (!tmp)
    return;
  *tmp = 0;
  s_arthcont acont = ARTHCONT(env, cont);
  s_arth_ast *ast = arth_parse(*str, &acont);
  *str = tmp + 2;
  char tab[12];
  int res = arth_exec(ast, &acont);
  sprintf(tab, "%d", res);
  for (char *c = tab; *c; c++)
    evect_push(vec, *c);
  arth_free(ast);
}
