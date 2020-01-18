#include <stdio.h>

#include "shparse/ast.h"

static inline void print_id(FILE *f, void *node)
{
    fprintf(f, "\"%p\"", node);
}

static inline void print_raw_rel(FILE *f, void *parent, void *child)
{
    print_id(f, parent);
    fprintf(f, " -> ");
    print_id(f, child);
}

static inline void print_stop(FILE *f)
{
    fprintf(f, ";\n");
}

static inline void print_label_start(FILE *f)
{
    fprintf(f, " [label=\"");
}

static inline void print_label_stop(FILE *f)
{
    fprintf(f, "\"]");
}

static inline void print_label(FILE *f, const char *label)
{
    print_label_start(f);
    fputs(label, f);
    print_label_stop(f);
}

static inline void print_rel(FILE *f, void *a, void *b, const char *label)
{
    print_raw_rel(f, a, b);
    if (label != NULL)
        print_label(f, label);
    print_stop(f);
}

static inline void print_node(FILE *f, void *node, const char *label)
{
    print_id(f, node);
    if (label != NULL)
        print_label(f, label);
    print_stop(f);
}

void for_print(FILE *f, struct shast *ast)
{
    struct shast_for *for_node = (struct shast_for *)ast;
    print_id(f, ast);
    print_label_start(f);
    fprintf(f, "FOR %s in", shword_buf(for_node->var));
    struct wordlist *wl = &for_node->collection;
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        fputc(' ', f);
        fputs(wordlist_get_str(wl, i), f);
    }
    print_label_stop(f);
    print_stop(f);

    ast_print_rec(f, for_node->body);
    print_rel(f, for_node, for_node->body, "DO");
}


void cmd_print(FILE *f, struct shast *ast)
{
    struct shast_cmd *command = (struct shast_cmd*)ast;
    print_id(f, ast);
    print_label_start(f);
    struct wordlist *wl = &command->arguments;
    fputs("CMD\\n", f);
    for (size_t i = 0; i < wordlist_size(wl); i++)
    {
        if (i > 0)
            fputc(' ', f);
        fprintf(f, "%s", wordlist_get_str(wl, i));
    }
    print_label_stop(f);
    print_stop(f);
}


void pipe_print(FILE *f, struct shast *ast)
{
    struct shast_pipe *pipe = (struct shast_pipe *)ast;
    print_node(f, pipe, "|");

    ast_print_rec(f, pipe->left);
    print_rel(f, pipe, pipe->left, NULL);

    ast_print_rec(f, pipe->right);
    print_rel(f, pipe, pipe->right, NULL);
}


void function_print(FILE *f, struct shast *ast)
{
    struct shast_function *function = (struct shast_function *)ast;
    print_node(f, function, hash_head_key(&function->hash));
    ast_print_rec(f, function->body);
    print_rel(f, function, function->body, NULL);
}


void list_print(FILE *f, struct shast *ast)
{
    struct shast_list *list = (struct shast_list *)ast;

    print_node(f, list, "LIST");

    for (size_t i = 0; i < shast_vect_size(&list->commands); i++) {
        struct shast *cur = shast_vect_get(&list->commands, i);
        ast_print_rec(f, cur);
        print_rel(f, list, cur, NULL);
    }
}

void if_print(FILE *f, struct shast *ast)
{
    struct shast_if *if_node = (struct shast_if *)ast;

    print_node(f, if_node, "IF");

    // print the condition
    ast_print_rec(f, if_node->condition);
    print_rel(f, if_node, if_node->condition, "COND");

    // print the true branch
    ast_print_rec(f, if_node->branch_true);
    print_rel(f, if_node, if_node->branch_true, "THEN");

    // print the false branch, if it exists
    if (if_node->branch_false) {
        ast_print_rec(f, if_node->branch_false);
        print_rel(f, if_node, if_node->branch_false, "ELSE");
    }
}

void while_print(FILE *f, struct shast *ast)
{
    struct shast_while *while_node = (struct shast_while *)ast;
    const char *struct_name = while_node->is_until ? "UNTIL" : "WHILE";
    print_node(f, while_node, struct_name);

    ast_print_rec(f, while_node->condition);
    print_rel(f, while_node, while_node->condition, "COND");

    ast_print_rec(f, while_node->body);
    print_rel(f, while_node, while_node->body, "DO");
}


void block_print(FILE *f, struct shast *node)
{
    struct shast_block *block = (struct shast_block *)node;
    print_node(f, block, "BLOCK");
    redir_vect_print(f, &block->redirs, node);
    assign_vect_print(f, &block->assigns, node);
    if (block->command) {
        ast_print_rec(f, block->command);
        print_rel(f, block, block->command, "CMD");
    }
}

void subshell_print(FILE *f, struct shast *ast)
{
    struct shast_subshell *subshell = (struct shast_subshell *)ast;
    print_node(f, subshell, "SUBSHELL");
    ast_print_rec(f, subshell->action);
    print_rel(f, subshell, subshell->action, NULL);
}

static void assignment_print(FILE *f, struct shast_assignment *assignment)
{
    print_id(f, assignment);
    print_label_start(f);
    fprintf(f, "%s = %s", assignment->name, assignment->value);
    print_label_stop(f);
    print_stop(f);
}

void assign_vect_print(FILE *f, struct assign_vect *vect, void *parent)
{
    for (size_t i = 0; i < assign_vect_size(vect); i++)
    {
        struct shast_assignment *assign = assign_vect_get(vect, i);
        assignment_print(f, assign);
        print_rel(f, parent, assign, "ASSIGN");
    }
}

void bool_op_print(FILE *f, struct shast *ast)
{
    struct shast_bool_op *bool_op = (struct shast_bool_op *)ast;
    print_node(f, bool_op, bool_op_to_string(bool_op->type));

    ast_print_rec(f, bool_op->left);
    print_rel(f, bool_op, bool_op->left, NULL);

    ast_print_rec(f, bool_op->right);
    print_rel(f, bool_op, bool_op->right, NULL);
}

void negate_print(FILE *f, struct shast *ast)
{
    struct shast_negate *negate = (struct shast_negate *)ast;
    print_node(f, negate, "!");

    ast_print_rec(f, negate->child);
    print_rel(f, negate, negate->child, NULL);
}

void case_print(FILE *f, struct shast *ast)
{
    struct shast_case *case_node = (struct shast_case *)ast;
    print_node(f, case_node, "CASE");
    for (size_t case_i = 0; case_item_vect_size(&case_node->cases); case_i++) {
        struct shast_case_item *case_item = case_item_vect_get(&case_node->cases, case_i);
        ast_print_rec(f, case_item->action);
        print_raw_rel(f, case_node, case_item->action);
        print_label_start(f);
        for (size_t i = 0; i < wordlist_size(&case_item->pattern); i++)
        {
            if (i > 0)
                fputc('|', f);
            fputs(wordlist_get_str(&case_item->pattern, i), f);
        }
        print_label_stop(f);
        print_stop(f);
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
