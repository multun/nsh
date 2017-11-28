#include "utils/alloc.h"
#include "utils/error.h"


s_sherror *sherror_alloc(s_lineinfo *lineinfo, char *message)
{
  s_sherror *res = xmalloc(sizeof(*res));
  res->lineinfo = lineinfo;
  res->message = message;
  return res;
}


s_sherror *sherror_free(s_sherror *error)
{
  free(error);
}
