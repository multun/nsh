#include <stdio.h>

#include "ast/ast.h"


void redirection_print(FILE *f, s_ast *ast)
{
  s_aredirection *aredirection = &ast->data.ast_redirection;
  void *id = ast;
  char *redir = "<";

  if (aredirection->type == REDIR_DLESS)
    redir = "<<";
  else if (aredirection->type == REDIR_GREAT)
    redir = ">";
  else if (aredirection->type == REDIR_DGREAT)
    redir = ">>";
  else if (aredirection->type == REDIR_LESSAND)
    redir = "<&";
  else if (aredirection->type == REDIR_GREATAND)
    redir = "<&";
  else if (aredirection->type == REDIR_LESSDASH)
    redir = "<-";
  else if (aredirection->type == REDIR_LESSGREAT)
    redir = "<>";
  else if (aredirection->type == REDIR_CLOBBER)
    redir = ">|";

  fprintf(f, "\"%p\" [label=\"%d %s %s\"];\n", id, aredirection->left,
          redir, aredirection->right->str);

  if (aredirection->action)
  {
    ast_print_rec(f, aredirection->action);
    void *id_next = aredirection->action;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
  }
}
