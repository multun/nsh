#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "shparse/ast.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"

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

void assignment_exec(struct environment *env, struct shast_assignment *assign, struct errcont *cont)
{
    char *name = strdup(assign->name);
    char *value = expand(&assign->line_info, assign->value, env, cont);
    environment_var_assign(env, name, value, false);
}

void assignment_free(struct shast_assignment *assign)
{
    if (!assign)
        return;

    free(assign->name);
    // don't free the value, as it's a pointer to the end of the key=value string
    free(assign);
}
