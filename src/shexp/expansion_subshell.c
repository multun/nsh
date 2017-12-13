#include <err.h>

#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/evect.h"


void expand_subshell(char **str, s_env *env, s_evect *vec)
{
  if (!*str && !env && !vec)
    warnx("expand_subshell: not implemented yet");
}


void expand_arth(char **str, s_env *env, s_evect *vec)
{
  if (!*str && !env && !vec)
    warnx("expand_arth: not implemented yet");
}
