#include "shexec/clean_exit.h"

s_ex_class g_clean_exit;


void clean_exit(s_errcont *cont, int retcode)
{
  cont->errman->retcode = retcode;
  shraise(cont, &g_clean_exit);
}
