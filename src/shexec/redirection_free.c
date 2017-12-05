#include "ast/ast.h"


void redirection_free(struct ast *ast)
{
  if (!ast)
    return;
  wordlist_free(ast->data.ast_redirection.right, true);
  ast_free(ast->data.ast_redirection.action);
  free(ast);
}
