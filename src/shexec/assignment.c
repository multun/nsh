#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "shparse/ast.h"
#include "shexec/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"

void assignment_exec(struct environment *env, struct shast_assignment *assign, struct ex_scope *ex_scope)
{
    char *name = strdup(assign->name);
    char *value = expand_nosplit(&assign->line_info, assign->value, EXP_FLAGS_ASSIGNMENT, env, ex_scope);
    environment_var_assign(env, name, &sh_string_create(value)->base, false);
}
