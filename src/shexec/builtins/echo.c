#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils/evect.h"
#include "shexec/builtins.h"


enum shecho_opt
{
  SHECHO_NL = 1,
  SHECHO_ESC = 2,
  SHECHO_RET = 4,
};


static void builtin_echo_parse_opt(int *opt, int argc, char **argv)
{
  int option = -1;
  optind = 1;
  for (bool running = true; running
       && (option = getopt(argc, argv, "+:neE"));)
    switch (option)
    {
    case 'n':
      *opt |= SHECHO_NL;
      break;
    case 'e':
      *opt |= SHECHO_ESC;
      break;
    case 'E':
      *opt &= ~SHECHO_ESC;
      break;
    default:
      running = false;
      break;
    }
}


static void echo_parse_base(char **c, int base, s_evect *vec)
{
  char res = 0;
  size_t index = 0;
  size_t len = base == 8 ? 3 : 2;
  for (; index < len && (*c)[1]; index++)
  {
    res *= base;
    if ((*c)[1] >= '0' && ((*c)[1] < '8' || ((*c)[1] <= '9' && base > 9)))
      res += ((*c)++)[1] - '0';
    else if (base == 16 && ((*c)[1] >= 'a' && ((*c)[1] < 'f')))
      res += ((*c)++)[1] - 'a';
    else if (base == 16 && ((*c)[1] >= 'A' && ((*c)[1] < 'F')))
      res += ((*c)++)[1] - 'A';
    else
      break;
  }
  if (index < 1 && base == 16)
  {
    evect_push(vec, '\\');
    evect_push(vec, 'x');
  }
  else if (index)
    evect_push(vec, res);
}


static void echo_print_num(char **c, s_evect *vec)
{
  switch (**c)
  {
  case 'r':
    evect_push(vec, '\r');
    break;
  case 't':
    evect_push(vec, '\t');
    break;
  case 'v':
    evect_push(vec, '\v');
    break;
  case '\\':
    evect_push(vec, '\\');
    break;
  case '0':
    echo_parse_base(c, 8, vec);
    break;
  case 'x':
    echo_parse_base(c, 16, vec);
    break;
  default:
    evect_push(vec, '\\');
    evect_push(vec, **c);
    break;
  }
}


static void echo_print_esc(char **c, s_evect *vec)
{
  switch (**c)
  {
  case 'a':
    evect_push(vec, '\a');
    break;
  case 'b':
    evect_push(vec, '\b');
    break;
  case 'c':
    evect_push(vec, '\0');
    break;
  case 'e': case 'E':
    evect_push(vec, 0x1b);
    break;
  case 'f':
    evect_push(vec, '\f');
    break;
  case 'n':
    evect_push(vec, '\n');
    break;
  default:
    echo_print_num(c, vec);
    break;
  }
}


static void echo_print(char *c, int *opt, s_evect *vec)
{
  bool esc = false;
  for (; *c && !(*opt & SHECHO_RET); c++)
  {
    if (!(*opt & SHECHO_ESC))
      evect_push(vec, *c);
    else if (!esc)
    {
      if (*c != '\\')
        evect_push(vec, *c);
      else
        esc = true;
    }
    else
    {
      esc = false;
      echo_print_esc(&c, vec);
    }
  }
  if (esc)
    evect_push(vec, '\\');
}


int builtin_echo(s_env *env, s_errcont *cont, int argc, char **argv)
{
  if (!env || !cont)
    warnx("cd: missing context elements");

  int opt = 0;
  builtin_echo_parse_opt(&opt, argc, argv);
  s_evect vec;
  if (argv[optind])
    evect_init(&vec, strlen(argv[optind]));
  else
    evect_init(&vec, 2);
  for (int i = optind; i < argc; i++)
  {
    if (i != optind)
      evect_push(&vec, ' ');
    echo_print(argv[i], &opt, &vec);
  }
  if (!(opt & SHECHO_NL))
    evect_push(&vec, '\n');
  evect_push(&vec, '\0');

  int res = printf("%s", vec.data) < 0;
  evect_destroy(&vec);
  return !!(res);
}
