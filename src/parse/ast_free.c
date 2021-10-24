#include <nsh_parse/ast.h>
#include <nsh_utils/macros.h>

void if_free(struct shast *ast)
{
    struct shast_if *if_node = (struct shast_if *)ast;
    shast_ref_put(if_node->condition);
    shast_ref_put(if_node->branch_true);
    shast_ref_put(if_node->branch_false);
    free(ast);
}

void while_free(struct shast *ast)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    shast_ref_put(while_node->condition);
    shast_ref_put(while_node->body);
    free(while_node);
}

void pipeline_free(struct shast *ast)
{
    struct shast_pipeline *pipe = (struct shast_pipeline *)ast;
    for (size_t i = 0; i < shast_vect_size(&pipe->children); i++)
        shast_ref_put(shast_vect_get(&pipe->children, i));
    shast_vect_destroy(&pipe->children);
    free(ast);
}

void list_free(struct shast *ast)
{
    struct shast_list *list = (struct shast_list *)ast;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        shast_ref_put(shast_vect_get(&list->commands, i));
    shast_vect_destroy(&list->commands);
    free(ast);
}

void subshell_free(struct shast *ast)
{
    struct shast_subshell *subshell = (struct shast_subshell *)ast;
    shast_ref_put(subshell->action);
    free(ast);
}

void bool_op_free(struct shast *ast)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    shast_ref_put(bool_op->left);
    shast_ref_put(bool_op->right);
    free(bool_op);
}

void negate_free(struct shast *ast)
{
    struct shast_negate *negate = (struct shast_negate *)ast;
    shast_ref_put(negate->child);
    free(negate);
}

static void case_item_free(struct shast_case_item *case_item)
{
    wordlist_destroy(&case_item->pattern);
    shast_ref_put(case_item->action);
    free(case_item);
}

void case_free(struct shast *ast)
{
    struct shast_case *case_node = (struct shast_case *)ast;

    free(case_node->var);
    for (size_t case_i = 0; case_i < case_item_vect_size(&case_node->cases); case_i++)
        case_item_free(case_item_vect_get(&case_node->cases, case_i));
    case_item_vect_destroy(&case_node->cases);
    free(ast);
}

void cmd_free(struct shast *ast)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    wordlist_destroy(&command->arguments);
    free(command);
}

void block_free(struct shast *ast)
{
    struct shast_block *block = (struct shast_block *)ast;
    for (size_t i = 0; i < redir_vect_size(&block->redirs); i++)
        shast_redirection_free(redir_vect_get(&block->redirs, i));
    redir_vect_destroy(&block->redirs);

    for (size_t i = 0; i < assign_vect_size(&block->assigns); i++)
        shast_assignment_free(assign_vect_get(&block->assigns, i));
    assign_vect_destroy(&block->assigns);

    shast_ref_put(block->command);
    free(ast);
}

void for_free(struct shast *ast)
{
    struct shast_for *for_node = (struct shast_for *)ast;
    free(for_node->var);
    wordlist_destroy(&for_node->collection);
    shast_ref_put(for_node->body);
    free(for_node);
}

void function_free(struct shast *ast)
{
    struct shast_function *func = (struct shast_function *)ast;
    free(hash_head_key(&func->hash));
    shast_ref_put(func->body);
    free(func);
}

#define AST_FREE_UTILS(EnumName, Name) \
    [EnumName] = Name ## _free,
static void (*ast_free_utils[])(struct shast *ast) =
{
    AST_TYPE_APPLY(AST_FREE_UTILS)
};

void ast_ref_free(struct refcnt *refcnt)
{
    struct shast *ast = container_of(refcnt, struct shast, refcnt);
    ast_free_utils[ast->type](ast);
}
