#include <assert.h>
#include <string.h>

#include "shparse/parse.h"
#include "utils/alloc.h"
#include "shlex/print.h"


bool start_redir(const s_token *tok)
{
  return tok_is(tok, TOK_IO_NUMBER) || tok_is(tok, TOK_DLESS)
         || tok_is(tok, TOK_DGREAT) || tok_is(tok, TOK_LESSAND)
         || tok_is(tok, TOK_GREATAND) || tok_is(tok, TOK_LESSGREAT)
         || tok_is(tok, TOK_LESSDASH) || tok_is(tok, TOK_CLOBBER)
         || tok_is(tok, TOK_LESS) || tok_is(tok, TOK_GREAT);
}


static s_ast *parse_assignment(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_ASSIGNMENT;
  s_token *tok = lexer_pop(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return res;
  char * val = strchr(TOK_STR(tok), '=');
  *val = '\0';
  val++;
  s_wordlist *name = xcalloc(sizeof(s_wordlist), 1);
  *name = WORDLIST(TOK_STR(tok), false, false, NULL);
  s_wordlist *value = xcalloc(sizeof(s_wordlist), 1);
  *value = WORDLIST(val, true, true, NULL);
  tok_free(tok, false);
  res->data.ast_assignment = AASSIGNMENT(name, value, NULL);
  return res;
}


typedef struct block_builder
{
  s_ablock *block;
  s_ast *assign;
  s_ast *redir;
  s_wordlist *elm;
} s_bbuilder;


static bool loop_redir(s_lexer *lexer, s_errman *errman, s_bbuilder *build)
{
  s_ast *tmp = parse_redirection(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  if (build->redir)
    build->redir->data.ast_redirection.action = tmp;
  else
    build->block->redir = tmp;
  build->redir = tmp;
  return true;
}


static bool prefix_loop(s_lexer *lexer, s_errman *errman, s_bbuilder *build)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  while (tok_is(tok, TOK_ASSIGNMENT_WORD) || start_redir(tok))
  {
    if (tok_is(tok, TOK_ASSIGNMENT_WORD))
    {
      s_ast *tmp = parse_assignment(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return false;
      if (build->assign)
        build->assign->data.ast_assignment.action = tmp;
      else
        build->block->def = tmp;
      build->assign = tmp;
    }
    else if (!loop_redir(lexer, errman, build))
      return false;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
  }
  return true;
}


static s_ast *asigne_cmd(s_wordlist *tmp)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_CMD;
  res->data.ast_cmd = ACMD(tmp);
  return res;
}


static bool element_loop(s_lexer *lexer, s_errman *errman, s_bbuilder *build)
{
  const s_token *tok = lexer_peek(lexer, errman);
  if (ERRMAN_FAILING(errman))
    return false;
  while (tok_is(tok, TOK_WORD) || start_redir(tok))
  {
    if (tok_is(tok, TOK_WORD))
    {
      s_wordlist *tmp = parse_word(lexer, errman);
      if (ERRMAN_FAILING(errman))
        return false;
      if (build->elm)
        build->elm->next = tmp;
      else
        build->block->cmd = asigne_cmd(tmp);
      build->elm = tmp;
    }
    else if (!loop_redir(lexer, errman, build))
      return false;
    tok = lexer_peek(lexer, errman);
    if (ERRMAN_FAILING(errman))
      return false;
  }
  return true;
}


s_ast *parse_simple_command(s_lexer *lexer, s_errman *errman)
{
  s_ast *res = xcalloc(sizeof(s_ast), 1);
  res->type = SHNODE_BLOCK;
  res->data.ast_block = ABLOCK(NULL, NULL, NULL);
  s_bbuilder builder =
  {
    .block = &res->data.ast_block,
    .assign = NULL,
    .redir = NULL,
    .elm = NULL,
  };
  if (!prefix_loop(lexer, errman, &builder)
      || !element_loop(lexer, errman, &builder))
    return res;
  if (!res->data.ast_block.redir && !res->data.ast_block.def
      && !res->data.ast_block.cmd)
  {
    const s_token *tok = lexer_peek(lexer, errman);
    assert(!ERRMAN_FAILING(errman));
    sherror(&tok->lineinfo, errman, "parsing error %s", TOKT_STR(tok));
    return res;
  }
  return res;
}
