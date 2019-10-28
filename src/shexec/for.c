#include <stdio.h>
#include <string.h>

#include "shparse/ast.h"
#include "shexec/break.h"
#include "shexec/environment.h"
#include "shexp/expansion.h"
#include "utils/alloc.h"

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

static void for_exception_handler(volatile bool *local_continue, struct errcont *cont,
                                  struct environment *env, size_t volatile *i)
{
    // the break builtin ensures no impossible break is emitted
    if (cont->errman->class != &g_lbreak || --env->break_count) {
        env->depth--;
        shraise(cont, NULL);
    }

    if ((*local_continue = env->break_continue))
        (*i)++;
}

int for_exec(struct environment *env, struct shast *ast, struct errcont *cont)
{
    struct shast_for *for_node = (struct shast_for *)ast;

    volatile int ret = 0;
    volatile bool local_continue = true;
    volatile size_t i = 0;

    env->depth++;
    struct keeper keeper = KEEPER(cont->keeper);
    struct errcont ncont = ERRCONT(cont->errman, &keeper);
    if (setjmp(keeper.env))
        for_exception_handler(&local_continue, cont, env, &i);

    struct wordlist *wl = &for_node->collection;
    if (local_continue)
        for (; i < wordlist_size(wl); i++) {
            char *var_value = expand(&ast->line_info, shword_buf(wordlist_get(wl, i)), env, cont);
            environment_var_assign(env, strdup(shword_buf(for_node->var)), var_value, false);
            ret = ast_exec(env, for_node->body, &ncont);
        }

    env->depth--;
    return ret;
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
