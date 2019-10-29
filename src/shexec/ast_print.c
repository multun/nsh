#include <stdio.h>

#include "shparse/ast.h"

void for_print(FILE *f, struct shast *ast)
{
    struct shast_for *for_node = (struct shast_for *)ast;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"FOR %s in", id, shword_buf(for_node->var));
    struct wordlist *wl = &for_node->collection;
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        if (i > 0)
            fputc(' ', f);
        fprintf(f, "%s", wordlist_get_str(wl, i));
    }
    fprintf(f, "\"];\n");
    ast_print_rec(f, for_node->body);
    void *id_do = for_node->body;
    fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n", id, id_do);
}


void cmd_print(FILE *f, struct shast *ast)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    struct wordlist *wl = &command->arguments;
    fprintf(f, "\"%p\" [label=\"CMD\\n", (void*)ast);
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        if (i > 0)
            fputc(' ', f);
        fprintf(f, "%s", wordlist_get_str(wl, i));
    }
    fprintf(f, "\"];\n");
}


void pipe_print(FILE *f, struct shast *ast)
{
    struct shast_pipe *pipe = (struct shast_pipe *)ast;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"|\"];\n", id);

    void *id_left = pipe->left;
    ast_print_rec(f, pipe->left);
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

    void *id_right = pipe->right;
    ast_print_rec(f, pipe->right);
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
}


void function_print(FILE *f, struct shast *ast)
{
    struct shast_function *function = (struct shast_function *)ast;
    fprintf(f, "\"%p\" [label=\"FUNC\n%s\"];\n", (void*)ast, hash_head_key(&function->hash));
    ast_print_rec(f, function->body);
    fprintf(f, "\"%p\" -> \"%p\";\n", (void*)ast, (void*)function->body);
}


void list_print(FILE *f, struct shast *ast)
{
    struct shast_list *list = (struct shast_list *)ast;

    void *id = ast;
    fprintf(f, "\"%p\" [label=\"LIST\"];\n", id);

    for (size_t i = 0; i < shast_vect_size(&list->commands); i++) {
        struct shast *cur = shast_vect_get(&list->commands, i);
        ast_print_rec(f, cur);
        void *id_cur = cur;
        fprintf(f, "\"%p\" -> \"%p\";\n", id, id_cur);
    }
}

void if_print(FILE *f, struct shast *ast)
{
    struct shast_if *if_node = (struct shast_if *)ast;

    // print the if label
    fprintf(f, "\"%p\" [label=\"IF\"];\n", (void*)ast);
    ast_print_rec(f, if_node->condition);
    fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n",
            (void*)ast, (void*)if_node->condition);

    // print the true branch
    ast_print_rec(f, if_node->branch_true);
    fprintf(f, "\"%p\" -> \"%p\" [label=\"THEN\"];\n",
            (void*)ast, (void*)if_node->branch_true);

    // print the false branch, if it exists
    if (if_node->branch_false) {
        ast_print_rec(f, if_node->branch_false);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"ELSE\"];\n",
                (void*)ast, (void*)if_node->branch_false);
    }
}

void while_print(FILE *f, struct shast *ast)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    const char *struct_name = while_node->is_until ? "UNTIL" : "WHILE";
    fprintf(f, "\"%p\" [label=\"%s\"];\n", (void*)ast, struct_name);

    ast_print_rec(f, while_node->condition);
    fprintf(f, "\"%p\" -> \"%p\" [label=\"COND\"];\n",
            (void*)ast, (void*)while_node->condition);

    ast_print_rec(f, while_node->body);
    fprintf(f, "\"%p\" -> \"%p\" [label=\"DO\"];\n",
            (void*)ast, (void*)while_node->body);
}


void block_print(FILE *f, struct shast *node)
{
    struct shast_block *block = (struct shast_block *)node;
    void *id = node;
    fprintf(f, "\"%p\" [label=\"BLOCK\"];\n", id);
    redir_vect_print(f, &block->redirs, node);
    assign_vect_print(f, &block->assigns, node);
    if (block->command) {
        ast_print_rec(f, block->command);
        void *id_next = block->command;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"CMD\"];\n", id, id_next);
    }
}

void subshell_print(FILE *f, struct shast *ast)
{
    struct shast_subshell *subshell = (struct shast_subshell *)ast;
    void *id = ast;
    fprintf(f, "\"%p\" [label=\"SUBSHELL\"];\n", id);
    void *id_next = subshell->action;
    ast_print_rec(f, subshell->action);
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_next);
}

static void assignment_print(FILE *f, struct shast_assignment *assignment)
{
    fprintf(f, "\"%p\" [label=\"%s = %s\"];\n", (void*)assignment, assignment->name,
            assignment->value);
}

void assign_vect_print(FILE *f, struct assign_vect *vect, void *parent)
{
    for (size_t i = 0; i < assign_vect_size(vect); i++)
    {
        struct shast_assignment *assign = assign_vect_get(vect, i);
        assignment_print(f, assign);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"ASSIGN\"];\n", parent, (void*)assign);
    }
}

void bool_op_print(FILE *f, struct shast *ast)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    void *id = ast;
    if (bool_op->type == BOOL_OR)
        fprintf(f, "\"%p\" [label=\"OR\"];\n", id);
    else if (bool_op->type == BOOL_AND)
        fprintf(f, "\"%p\" [label=\"AND\"];\n", id);
    else
        fprintf(f, "\"%p\" [label=\"NOT\"];\n", id);

    ast_print_rec(f, bool_op->left);
    void *id_left = bool_op->left;
    fprintf(f, "\"%p\" -> \"%p\";\n", id, id_left);

    if (bool_op->type != BOOL_NOT) {
        ast_print_rec(f, bool_op->right);
        void *id_right = bool_op->right;
        fprintf(f, "\"%p\" -> \"%p\";\n", id, id_right);
    }
}

void case_print(FILE *f, struct shast *ast)
{
    struct shast_case *case_node = (struct shast_case *)ast;
    fprintf(f, "\"%p\" [label=\"CASE\"];\n", (void*)ast);
    for (size_t case_i = 0; case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        ast_print_rec(f, case_item->action);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"", (void*)ast, (void*)case_item->action);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++)
        {
            if (i > 0)
                fputc('|', f);
            fprintf(f, "%s", wordlist_get_str(&case_item->pattern, i));
        }
        fprintf(f, "\"];\n");
    }
}

#define AST_PRINT_UTILS(EnumName, Name) \
    [EnumName] = Name ## _print,
static void (*ast_print_utils[])(FILE *f, struct shast *ast) =
{
    AST_TYPE_APPLY(AST_PRINT_UTILS)
};

void ast_print_rec(FILE *f, struct shast *ast)
{
    if (ast)
        ast_print_utils[ast->type](f, ast);
}

void ast_print(FILE *f, struct shast *ast)
{
    fprintf(f, "digraph G {\n");
    ast_print_rec(f, ast);
    fprintf(f, "}\n");
}
