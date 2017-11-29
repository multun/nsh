#include <stdio.h>

#include "ast/ast.h"
#include "ast/wordlist.h"
#include "ast/cmd.h"


void ast_print_test()
{
  s_ast nodecond = AST_ACMD(&WL("$test"));
  s_ast nodethen = AST_AWHILE(&AST_ACMD(&WL("true")), &AST_ACMD(&WL("echo forever")));
  s_ast nodefordo = AST_ACMD(&WL("echo $i"));
  s_ast nodefor = AST_AFOR(&WL("i"), &WL("$(seq 1 10)"), &nodefordo);
  s_ast root = AST_AIF(&nodecond, &nodethen, &nodefor);

  ast_print(stdout, &root);
}
