#include <stdbool.h>

#include "utils/evect.h"
#include "utils/alloc.h"
#include "shexp/arth.h"

bool is_arth_op(char c)
{
  return c == '-' || c == '+' || c == '*' || c == '/' || c == '&' || c == '|'
         || c == '!' || c == '~';
}


static void parse_arth_subshell(char **str, s_evect *vec)
{
  size_t nb_par = 1;
  evect_push(vec, *((*str)++));
  evect_push(vec, *((*str)++));
  //TODO: quotes in subshell
  while (nb_par)
  {
    char c = *((*str)++);
    if (c =='(')
      nb_par++;
    else if (c ==')')
      nb_par--;
    evect_push(vec, c);
  }
}


static void parse_arth_word(char **str, s_evect *vec)
{
  if (**str == '$' && (*str)[1] == '(')
    parse_arth_subshell(str, vec);
  else if (is_arth_op(**str))
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
    } while (**str && !is_arth_op(**str) && **str != ' ' && **str != '\n')
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

  return vec.value;
}


char **arth_lex(char *str)
{
  size_t nb = 0;
  char **elms = xcalloc(1, sizeof(char *));
  char *elm
  while ((elm = arth_lex_pop(&str)))
  {
    nb++;
    elms = xrealloc(elms, sizeof(char *) * (nb + 1));
    elms[nb - 1] = elm;
  }
  elms[nb] = NULL;
  return elms;
}
