#include "cli/cmdopts.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/error.h"



static void try_re(int *res, s_lexer *lex,
                   s_errcont *errcont, s_context *cont)
{
  parse(&cont->ast, lex, errcont);
  if (cont->ast)
  {
    if (g_cmdopts.ast_print)
    {
      FILE *f = fopen("42sh_ast.dot", "w+");
      ast_print(f, cont->ast);
      fclose(f);
    }

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
  *res = g_cmdopts.src == SHSRC_COMMAND ? 1 : 2;
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


int repl(s_context *ctx)
{
  int res = 0;

  for (bool stopping = false; !stopping && !cstream_eof(ctx->ms.cs);)
  {
    s_lexer *lex = lexer_create(ctx->ms.cs);
    stopping = ast_exec_consumer(&res, lex, ctx);
    lexer_free(lex);
  }

  return res;
}
