#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "shparse/ast.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"

void assignment_exec(struct environment *env, struct shast_assignment *assign, struct errcont *cont)
{
    char *name = strdup(assign->name);
    char *value = expand(&assign->line_info, assign->value, env, cont);
    environment_var_assign(env, name, value, false);
}
