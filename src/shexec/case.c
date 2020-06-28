#include "shexp/expansion.h"
#include "shparse/ast.h"

#include <stdio.h>
#include <string.h>
#include <fnmatch.h>

int case_exec(struct environment *env, struct shast *ast, struct ex_scope *ex_scope)
{
    struct shast_case *case_node = (struct shast_case *)ast;
    char *case_var = expand_nosplit(&case_node->base.line_info, shword_buf(case_node->var), 0, env, ex_scope);
    for (size_t case_i = 0; case_i < case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++)
        {
            char *pattern = shword_buf(wordlist_get(&case_item->pattern, i));
            if (fnmatch(pattern, case_var, 0) != 0)
                continue;

            free(case_var);
            return ast_exec(env, case_item->action, ex_scope);
        }
    }
    free(case_var);
    return 0;
}
