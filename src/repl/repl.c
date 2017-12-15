#include "cli/cmdopts.h"
#include "cli/shopt.h"
#include "repl/history.h"
#include "repl/repl.h"
#include "shexec/clean_exit.h"
#include "shlex/lexer.h"
#include "shparse/parse.h"
#include "utils/error.h"


static void try_re(s_lexer *lex, s_errcont *errcont, s_context *cont)
{
  parse(&cont->ast, lex, errcont);
  if (cont->ast)
  {
    if (g_shopts[SHOPT_AST_PRINT])
    {
      FILE *f = fopen("42sh_ast.dot", "w+");
      ast_print(f, cont->ast);
      fclose(f);
    }

    cont->env->code = ast_exec(cont->env, cont->ast, errcont);
    history_update(cont);
  }
}


static bool handle_rep_fail(s_errman *eman, s_context *cont)
{
  if (eman->class == &g_clean_exit)
  {
    cont->env->code = eman->retcode;
    return true;
  }
  cont->env->code = g_cmdopts.src == SHSRC_COMMAND ? 1 : 2;
  return !cont->cs->interactive;
}


static bool ast_exec_consumer(s_lexer *lex, s_context *cont)
{
  cont->line_start = true;
  cont->ast = NULL;

  s_errman eman = ERRMAN;
  s_keeper keeper = KEEPER(NULL);

  volatile bool stopping = false;
  if (setjmp(keeper.env))
  {
    if (handle_rep_fail(&eman, cont))
      stopping = true;
  }
  else
    try_re(lex, &ERRCONT(&eman, &keeper), cont);

  cont->ast_list = ast_list_append(cont->ast_list, cont->ast);
  return stopping;
}


bool repl(s_context *ctx)
{
  bool stopping = false;
  while (!stopping && !cstream_eof(ctx->cs))
  {
    s_lexer *lex = lexer_create(ctx->cs);
    stopping = ast_exec_consumer(lex, ctx);
    lexer_free(lex);
  }

  return stopping;
}
