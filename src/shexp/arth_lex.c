#include <stdbool.h>

#include "utils/evect.h"
#include "utils/alloc.h"
#include "shexp/arth.h"

bool is_arth_op(char c)
{
  return c == '-' || c == '+' || c == '*' || c == '/' || c == '&' || c == '|'
         || c == '!' || c == '~';
}


static void parse_arth_word(char **str, s_evect *vec)
{
  if (is_arth_op(**str))
  {
    char c = *((*str)++);
    evect_push(vec, c);
    if (**str == c)
    {
      evect_push(vec, c);
      (*str)++;
    }
  }
  else
    do {
      evect_push(vec, *((*str)++));
    } while (**str && !is_arth_op(**str) && **str != '\t' && **str != ' '
             && **str != '\n');
  evect_push(vec, '\0');
}


static char *arth_lex_pop(char **str)
{
  while (**str == ' ')
    (*str)++;
  if (!**str)
    return NULL;
  s_evect vec;
  evect_init(&vec, 10);

  parse_arth_word(str, &vec);

  return vec.data;
}


char **arth_lex(char *str, char ***end)
{
  size_t nb = 0;
  char **elms = xcalloc(1, sizeof(char *));
  char *elm;
  while ((elm = arth_lex_pop(&str)))
  {
    nb++;
    elms = xrealloc(elms, sizeof(char *) * (nb + 1));
    elms[nb - 1] = elm;
  }
  elms[nb] = NULL;
  *end = elms + nb;
  return elms;
}
