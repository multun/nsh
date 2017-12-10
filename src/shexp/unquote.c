#include "shexp/unquote.h"
#include "utils/evect.h"

#include <assert.h>
#include <string.h>
#include <stddef.h>


size_t unquote_simple(s_evect *v, const char *str)
{
  size_t i = 1; // skip the quote
  for (; str[i] != '\''; i++)
  {
    assert(str[i]);
    evect_push(v, str[i]);
  }
  return i + 1;
}


size_t unquote_double(s_evect *v, const char *str)
{
  size_t i = 1; // skip the quote
  for (; str[i] != '"'; i++)
  {
    if (str[i] == '\\')
    {
      assert(str[++i]);
      evect_push(v, str[i]);
      continue;
    }

    assert(str[i]);
    evect_push(v, str[i]);
  }
  return i + 1;
}


char *unquote(const char *str)
{
  s_evect res;
  evect_init(&res, strlen(str));
  size_t cdiff = 0;

  for (; *str; str += cdiff)
    switch (*str)
    {
    case '\'':
      cdiff = unquote_simple(&res, str);
      break;
    case '"':
      cdiff = unquote_double(&res, str);
      break;
    case '\\': default:
      if (*str == '\\')
      {
        str++;
        assert(*str);
      }

      evect_push(&res, *str);
      cdiff = 1;
      break;
    }
  evect_push(&res, '\0');
  return res.data;
}
