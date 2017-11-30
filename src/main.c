#include "io/cstream.h"
#include "shlex/lexer.h"
#include "shlex/print.h"

#include <stdio.h>


int main(int argc, char *argv[])
{
  FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;

  s_cstream cs;
  cstream_init_file(&cs, f, argv[1]);
  s_lexer *lex = lexer_create(&cs);
  while (!cstream_eof(&cs))
  {
    s_token *tok = lexer_pop(lex);
    if (!tok)
      break;

    printf("%s(%s)[%c]\n", token_type_to_string(tok->type),
           TOK_STR(tok), tok->delim);
    tok_free(tok, true);
  }
  lexer_free(lex);
  if (argc > 1)
    fclose(f);
  return 0;
}
