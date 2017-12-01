#include "cli/cmdopts.h"
#include "io/cstream.h"
#include "shlex/lexer.h"
#include "shlex/print.h"
#include "gen/config.h"

#include <stdio.h>
#include <err.h>


static int print_tokens(s_cstream *cs)
{
  s_lexer *lex = lexer_create(cs);
  while (!cstream_eof(cs))
  {
    s_token *tok = lexer_pop(lex);
    if (!tok)
      break;

    printf("%zu:%zu\t%s(%s)[%c]\n", tok->lineinfo.line, tok->lineinfo.column,
           token_type_to_string(tok->type),
           TOK_STR(tok), tok->delim);
    tok_free(tok, true);
  }
  lexer_free(lex);
  return 0;
}


static s_cstream *load_stream(int argc, char *argv[])
{
  if (g_cmdopts.src == SHSRC_COMMAND)
  {
    if (argc < 1)
      errx(1, "missing source");
    return cstream_from_string(argv[0], "<command line>");
  }
  else
  {
    FILE *f = argc > 0 ? fopen(argv[0], "r") : stdin;
    return cstream_from_file(f, argv[1]);
  }
}


int main(int argc, char *argv[])
{
  int cmdstart = cmdopts_parse(argc, argv);
  if (cmdstart < 0)
    // exit when cmdstart < 0,
    // but succeed when cmdstart == -1
    return cmdstart + 1;

  argc -= cmdstart;
  argv += cmdstart;

  if (g_cmdopts.norc)
    puts("norc");

  s_cstream *cs = load_stream(argc, argv);

  int res = 0;
  switch (g_cmdopts.shmode)
  {
  case SHMODE_VERSION:
    puts("Version " VERSION);
    res = 0;
    break;
  case SHMODE_AST_PRINT:
    res = 1;
    break;
  case SHMODE_TOKEN_PRINT:
    res = print_tokens(cs);
    break;
  case SHMODE_REGULAR:
    res = 3;
    break;
  }

  cstream_free(cs);
  return res;
}
