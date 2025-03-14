#include <err.h>
#include <string.h>

#include <nsh_exec/environment.h>
#include <nsh_exec/expansion.h>
#include <nsh_lex/variable.h>
#include <nsh_utils/hashmap_iterators.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/macros.h>

static int export_var(int *retcode, struct environment *env, char *raw_export_expr,
                      bool exported)
{
    nsh_err_t err;

    // expand the variable name
    char *export_expr;
    if ((err = expand_nosplit(&export_expr, NULL, raw_export_expr, env, 0)))
        return err;

    char *var_sep = strchr(export_expr, '=');
    char *var_name_end = var_sep;
    if (var_name_end == NULL)
        var_name_end = export_expr + strlen(export_expr);

    // validate the variable name
    size_t var_name_len = var_name_end - export_expr;

    if (variable_name_check_string(export_expr, var_name_len) != 0) {
        warnx("export: invalid identifier");
        free(export_expr);
        *retcode = 1;
        return NSH_OK;
    }

    // allocate new values
    struct sh_value *var_value = NULL;
    if (var_sep)
        var_value = &sh_string_create(strdup(var_sep + 1))->base;
    char *var_name = strndup(export_expr, var_name_len);
    free(export_expr);

    struct shexec_variable *var;
    struct hashmap_item **insertion_pos;
    struct hashmap_item *hash = hashmap_find(&env->variables, &insertion_pos, var_name);
    if (hash == NULL) {
        var = zalloc(sizeof(*var));
        hashmap_item_init(&var->hash, var_name);
        var->exported = exported;
        var->value = var_value;
        hashmap_insert(&env->variables, insertion_pos, &var->hash);
    } else {
        free(var_name);
        var = container_of(hash, struct shexec_variable, hash);
        var->exported = exported;
        /* only replace the value if we have one*/
        if (var_value) {
            sh_value_put(var->value);
            var->value = var_value;
        }
    }
    return NSH_OK;
}


static void export_print(struct environment *env)
{
    struct hashmap_it it;
    for_each_hash(it, &env->variables)
    {
        struct shexec_variable *var = container_of(it.cur, struct shexec_variable, hash);
        if (!var->exported)
            continue;

        const char *var_name = var->hash.key;
        if (var->value) {
            struct sh_value *value = var->value;
            if (!sh_value_is_string(value))
                continue;
            struct sh_string *string = (struct sh_string *)value;
            printf("export %s=\"%s\"\n", var_name, sh_string_data(string));
        } else
            printf("export %s\n", var_name);
    }
}

nsh_err_t builtin_export(struct environment *env, int argc, char **argv)
{
    nsh_err_t err;
    int retcode = 0;
    bool print = true;
    bool exported = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp("-n", argv[i]) == 0)
            exported = false;
        else if (strcmp("-p", argv[i])) {
            print = false;

            if ((err = export_var(&retcode, env, argv[i], exported)))
                return err;
        }
    }

    if (print)
        export_print(env);

    env->code = retcode;
    return NSH_OK;
}
