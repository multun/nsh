#include <stdio.h>

#include "ast/ast.h"
#include "shexec/environment.h"


void ast_print_test()
{
  s_ast nodecond = AST_ABOOL_OP(BOOL_AND, &AST_ACMD(&WL("$test")),
                                &AST_ACMD(&WL("$test2")));
  s_ast nodethen = AST_AWHILE(&AST_ACMD(&WL("true")),
                              &AST_ACMD(&WL("echo forever")));
  s_ast nodefordo = AST_ACMD(&WL("echo $i"));
  s_ast nodefor = AST_AFOR(&WL("i"), &WL("$(seq 1 10)"), &nodefordo);
  s_ast root = AST_AIF(&nodecond, &nodethen, &nodefor);

  ast_print(stdout, &root);
}


void ast_exec_test()
{
  s_ast root = AST_ACMD(&WL("/usr/bin/tree"));
  s_env *env = environment_create();
  ast_exec(env, &root);
  environment_free(env);
}
