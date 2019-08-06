#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "ast/ast.h"
#include "shexp/expansion.h"
#include "shexec/variable.h"
#include "utils/alloc.h"

void assignment_print(FILE *f, s_ast *ast)
{
    s_aassignment *aassignment = &ast->data.ast_assignment;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"%s = %s\"];\n", id, aassignment->name->str,
            aassignment->value->str);
    void *id_next = aassignment->action;
    ast_print_rec(f, aassignment->action);
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
}

int assignment_exec(s_env *env, s_ast *ast, s_ast *cmd, s_errcont *cont)
{
    if (!ast)
        return ast_exec(env, cmd, cont);
    char *name = strdup(ast->data.ast_assignment.name->str);
    char *value = expand(&ast->line_info, ast->data.ast_assignment.value->str, env, cont);
    bool valid =
        *name == '_' || (*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z');

    for (char *c = name + 1; valid && *c; c++)
        valid = *c == '_' || (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')
            || (*c >= '0' && *c <= '9');

    if (!valid) {
        warnx("assignment: '%s': not a valid identifier", name);
        free(value);
        free(name);
        return 127;
    }

    environment_var_assign(env, name, value, cmd != NULL);
    return assignment_exec(env, ast->data.ast_assignment.action, cmd, cont);
}

void assignment_free(struct ast *ast)
{
    if (!ast)
        return;
    wordlist_free(ast->data.ast_assignment.name, true);
    wordlist_free(ast->data.ast_assignment.value, false);
    ast_free(ast->data.ast_assignment.action);
    free(ast);
}
