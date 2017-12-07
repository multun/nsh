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


static void parse_assignment(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  *res = xcalloc(sizeof(s_ast), 1);
  (*res)->type = SHNODE_ASSIGNMENT;
  s_token *tok = lexer_pop(lexer, errcont);
  char * val = strchr(TOK_STR(tok), '=');
  *val = '\0';
  val++;
  s_wordlist *name = xcalloc(sizeof(s_wordlist), 1);
  *name = WORDLIST(TOK_STR(tok), false, false, NULL);
  s_wordlist *value = xcalloc(sizeof(s_wordlist), 1);
  *value = WORDLIST(val, true, true, NULL);
  tok_free(tok, false);
  (*res)->data.ast_assignment = AASSIGNMENT(name, value, NULL);
}


// TODO: fix obsolete architecture
typedef struct block_builder
{
  s_ablock *block;
  s_ast *assign;
  s_ast *redir;
  s_wordlist *elm;
} s_bbuilder;


static bool loop_redir(s_lexer *lexer, s_errcont *errcont, s_bbuilder *build)
{
  s_ast **target;
  if (build->redir)
    target = &build->redir->data.ast_redirection.action;
  else
    target = &build->block->redir;
  parse_redirection(target, lexer, errcont);
  build->redir = *target;
  return true;
}


static bool prefix_loop(s_lexer *lexer, s_errcont *errcont, s_bbuilder *build)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  while (tok_is(tok, TOK_ASSIGNMENT_WORD) || start_redir(tok))
  {
    if (tok_is(tok, TOK_ASSIGNMENT_WORD))
    {
      s_ast **target;
      if (build->assign)
        target = &build->assign->data.ast_assignment.action;
      else
        target = &build->block->def;
      parse_assignment(target, lexer, errcont);
      build->assign = *target;
    }
    else if (!loop_redir(lexer, errcont, build))
      return false;
    tok = lexer_peek(lexer, errcont);
  }
  return true;
}


static bool element_loop(s_lexer *lexer, s_errcont *errcont, s_bbuilder *build)
{
  const s_token *tok = lexer_peek(lexer, errcont);
  while (tok_is(tok, TOK_WORD) || start_redir(tok))
  {
    if (tok_is(tok, TOK_WORD))
    {
      s_wordlist **target;
      if (build->elm)
        target = &build->elm->next;
      else
      {
        build->block->cmd = xcalloc(sizeof(s_ast), 1);
        build->block->cmd->type = SHNODE_CMD;
        target = &build->block->cmd->data.ast_cmd.wordlist;
      }
      parse_word(target, lexer, errcont);
      build->elm = *target;
    }
    else if (!loop_redir(lexer, errcont, build))
      return false;
    tok = lexer_peek(lexer, errcont);
  }
  return true;
}


void parse_simple_command(s_ast **res, s_lexer *lexer, s_errcont *errcont)
{
  *res = xcalloc(sizeof(s_ast), 1);
  (*res)->type = SHNODE_BLOCK;
  (*res)->data.ast_block = ABLOCK(NULL, NULL, NULL);
  s_bbuilder builder =
  {
    .block = &(*res)->data.ast_block,
    .assign = NULL,
    .redir = NULL,
    .elm = NULL,
  };

  if (!prefix_loop(lexer, errcont, &builder)
      || !element_loop(lexer, errcont, &builder))
    return;

  if (!(*res)->data.ast_block.redir && !(*res)->data.ast_block.def
      && !(*res)->data.ast_block.cmd)
  {
    const s_token *tok = lexer_peek(lexer, errcont);
    sherror(&tok->lineinfo, errcont, "parsing error %s", TOKT_STR(tok));
  }
}
