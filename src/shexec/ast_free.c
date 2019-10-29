#include "shparse/ast.h"


void if_free(struct shast *ast)
{
    struct shast_if *if_node = (struct shast_if *)ast;
    if (!ast)
        return;
    ast_free(if_node->condition);
    ast_free(if_node->branch_true);
    ast_free(if_node->branch_false);
    free(ast);
}

void while_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_while *while_node = (struct shast_while *)ast;
    ast_free(while_node->condition);
    ast_free(while_node->body);
    free(while_node);
}

void pipe_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_pipe *pipe = (struct shast_pipe *)ast;
    ast_free(pipe->left);
    ast_free(pipe->right);
    free(ast);
}

void list_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_list *list = (struct shast_list *)ast;
    for (size_t i = 0; i < shast_vect_size(&list->commands); i++)
        ast_free(shast_vect_get(&list->commands, i));
    shast_vect_destroy(&list->commands);
    free(ast);
}

void subshell_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_subshell *subshell = (struct shast_subshell *)ast;
    ast_free(subshell->action);
    free(ast);
}

void bool_op_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    ast_free(bool_op->left);
    ast_free(bool_op->right);
    free(bool_op);
}

static void case_item_free(struct shast_case_item *case_item)
{
    wordlist_destroy(&case_item->pattern);
    ast_free(case_item->action);
    free(case_item);
}

void case_free(struct shast *ast)
{
    if (!ast)
        return;
    struct shast_case *case_node = (struct shast_case *)ast;

    free(case_node->var);
    for (size_t case_i = 0; case_i < case_item_vect_size(&case_node->cases); case_i++)
        case_item_free(case_item_vect_get(&case_node->cases, case_i));
    case_item_vect_destroy(&case_node->cases);
    free(ast);
}

void cmd_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_cmd *command = (struct shast_cmd*)ast;
    wordlist_destroy(&command->arguments);
    free(command);
}

void block_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_block *block = (struct shast_block *)ast;
    for (size_t i = 0; i < redir_vect_size(&block->redirs); i++)
        shast_redirection_free(redir_vect_get(&block->redirs, i));
    redir_vect_destroy(&block->redirs);

    for (size_t i = 0; i < assign_vect_size(&block->assigns); i++)
        shast_assignment_free(assign_vect_get(&block->assigns, i));
    assign_vect_destroy(&block->assigns);

    ast_free(block->command);
    free(ast);
}

void for_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_for *for_node = (struct shast_for *)ast;
    free(for_node->var);
    wordlist_destroy(&for_node->collection);
    ast_free(for_node->body);
    free(for_node);
}

void function_free(struct shast *ast)
{
    if (!ast)
        return;

    struct shast_function *func = (struct shast_function *)ast;
    ref_put(&func->refcnt);
}

void shast_function_ref_free(struct refcnt *refcnt)
{
    struct shast_function *func;
    func = container_of(refcnt, struct shast_function, refcnt);
    free(hash_head_key(&func->hash));
    ast_free(func->body);
    free(func);
}

void assignment_free(struct shast_assignment *assign)
{
    if (!assign)
        return;

    free(assign->name);
    // don't free the value, as it's a pointer to the end of the key=value string
    free(assign);
}

#define AST_FREE_UTILS(EnumName, Name) \
    [EnumName] = Name ## _free,
static void (*ast_free_utils[])(struct shast *ast) =
{
    AST_TYPE_APPLY(AST_FREE_UTILS)
};

void ast_free(struct shast *ast)
{
    if (!ast)
        return;
    ast_free_utils[ast->type](ast);
}
