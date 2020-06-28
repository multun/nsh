#include <err.h>
#include <string.h>

#include "shexec/builtins.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"
#include "utils/macros.h"

static int export_var(struct environment *env, char *raw_export_expr, bool exported, struct ex_scope *ex_scope)
{
    // expand the variable name
    char *export_expr = expand_nosplit(NULL, raw_export_expr, 0, env, ex_scope);
    char *var_sep = strchr(export_expr, '=');
    char *var_name_end = var_sep;
    if (var_name_end == NULL)
        var_name_end = export_expr + strlen(export_expr);

    // validate the variable name
    size_t var_name_len = var_name_end - export_expr;

    if (variable_name_check_string(export_expr, var_name_len) != 0)
    {
        warnx("export: invalid identifier");
        free(export_expr);
        return 1;
    }

    // allocate new values
    struct sh_value *var_value = NULL;
    if (var_sep)
        var_value = &sh_string_create(strdup(var_sep + 1))->base;
    char *var_name = strndup(export_expr, var_name_len);
    free(export_expr);

    struct shexec_variable *var;
    struct hash_head **insertion_pos;
    struct hash_head *hash = hash_table_find(&env->variables, &insertion_pos, var_name);
    if (hash == NULL)
    {
        var = zalloc(sizeof(*var));
        hash_head_init(&var->hash, var_name);
        var->exported = exported;
        var->value = var_value;
        hash_table_insert(&env->variables, insertion_pos, &var->hash);
    }
    else
    {
        free(var_name);
        var = container_of(hash, struct shexec_variable, hash);
        var->exported = exported;
        sh_value_put(var->value);
        var->value = var_value;
    }
    return 0;
}


static void export_print(struct environment *env)
{
    struct hash_table_it it;
    for_each_hash(it, &env->variables)
    {
        struct shexec_variable *var = container_of(it.cur, struct shexec_variable, hash);
        if (!var->exported)
            continue;

        const char *var_name = hash_head_key(&var->hash);
        if (var->value) {
            struct sh_value *value = var->value;
            if (!sh_value_is_string(value))
                continue;
            struct sh_string *string = (struct sh_string*)value;
            printf("export %s=\"%s\"\n", var_name, sh_string_data(string));
        }
        else
            printf("export %s\n", var_name);
    }
}

int builtin_export(struct environment *env, struct ex_scope *ex_scope, int argc, char **argv)
{
    int res = 0;
    bool print = true;
    bool exported = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp("-n", argv[i]) == 0)
            exported = false;
        else if (strcmp("-p", argv[i])) {
            print = false;
            res |= export_var(env, argv[i], exported, ex_scope);
        }
    }
    if (print)
        export_print(env);
    return res;
}
