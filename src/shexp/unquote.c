#include "shexec/clean_exit.h"
#include "shexp/unquote.h"
#include "utils/evect.h"

#include <assert.h>
#include <string.h>
#include <stddef.h>


static void unquote_assert(s_errcont *cont, bool test)
{
  clean_assert(cont, test, "unexpectedly reached the end "
               "of the unquoted string");
}


size_t unquote_simple(s_errcont *cont, s_evect *v, const char *str)
{
  size_t i = 1; // skip the quote
  for (; str[i] != '\''; i++)
  {
    unquote_assert(cont, str[i]);
    evect_push(v, str[i]);
  }
  return i + 1;
}


size_t unquote_double(s_errcont *cont, s_evect *v, const char *str)
{
  size_t i = 1; // skip the quote
  for (; str[i] != '"'; i++)
  {
    if (str[i] == '\\')
    {
      unquote_assert(cont, str[++i]);
      if (str[i] != '"')
        evect_push(v, '\\');
      evect_push(v, str[i]);
      continue;
    }

    unquote_assert(cont, str[i]);
    evect_push(v, str[i]);
  }
  return i + 1;
}




static void unquote_sub(s_evect *res, const char *str, s_errcont *cont)
{
  size_t cdiff = 0;

  for (; *str; str += cdiff)
    switch (*str)
    {
    case '\'':
      cdiff = unquote_simple(cont, res, str);
      break;
    case '"':
      cdiff = unquote_double(cont, res, str);
      break;
    case '\\': default:
      if (*str == '\\')
      {
        str++;
        unquote_assert(cont, *str);
      }

      evect_push(res, *str);
      cdiff = 1;
      break;
    }
  evect_push(res, '\0');
}



void unquote(char **sres, const char *str, s_errcont *cont)
{
  s_evect res;
  evect_init(&res, strlen(str) + 1);

  // the vector will never grow because of the above initial size
  *sres = res.data;
  unquote_sub(&res, str, cont);
}
