#include "utils/error.h"
#include "shlex/lexer.h"
#include "shexec/clean_exit.h"
#include "shparse/parse.h"
#include "repl/repl.h"
#include "repl/history.h"


static void try_re(int *res, s_lexer *lex,
                   s_errcont *errcont, s_context *cont)
{
  parse(&cont->ast, lex, errcont);
  if (cont->ast)
  {
    *res = ast_exec(cont->env, cont->ast, errcont);
    history_update(cont);
  }
}


static bool handle_rep_fail(int *res, s_errman *eman)
{
  if (eman->class == &g_clean_exit)
  {
    *res = eman->retcode;
    return true;
  }
  *res = 2;
  return false;
}


static bool ast_exec_consumer(int *res, s_lexer *lex, s_context *cont)
{
  cont->line_start = true;
  cont->ast = NULL;

  s_errman eman = ERRMAN;
  s_keeper keeper = KEEPER(NULL);

  bool stopping = false;
  if (setjmp(keeper.env))
  {
    if (handle_rep_fail(res, &eman))
      stopping = true;
  }
  else
    try_re(res, lex, &ERRCONT(&eman, &keeper), cont);

  cont->ast_list = ast_list_append(cont->ast_list, cont->ast);
  return stopping;
}


int producer(s_context *ctx, int argc, char *argv[])
{
  managed_stream_init(ctx, &ctx->ms, argc, argv);
  history_init(ctx);

  int res = 0;
  for (bool stopping = false; !stopping && !cstream_eof(ctx->ms.cs);)
  {
    s_lexer *lex = lexer_create(ctx->ms.cs);
    stopping = ast_exec_consumer(&res, lex, ctx);
    lexer_free(lex);
  }

  managed_stream_destroy(&ctx->ms);
  return res;
}
