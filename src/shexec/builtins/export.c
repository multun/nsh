#include <err.h>
#include <string.h>

#include "ast/assignment.h"
#include "shexec/builtins.h"
#include "shexp/expansion.h"
#include "shlex/variable.h"
#include "utils/alloc.h"

static void unexport_var(struct environment *env, char *name)
{
    struct pair *prev = htable_access(env->vars, name);
    if (prev) {
        struct variable *var = prev->value;
        var->to_export = false;
    }
    free(name);
}

static int export_var(struct environment *env, char *entry, bool remove, struct errcont *cont)
{
    int res = 0;
    char *var = expand(NULL, entry, env, cont);
    char *word = NULL;
    char *name = strdup(strtok_r(var, "=", &word));
    bool valid =
        *name == '_' || (*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z');

    for (char *c = name + 1; valid && *c; c++)
        valid = *c == '_' || (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')
            || (*c >= '0' && *c <= '9');

    if (entry[0] == '=' || !valid) {
        warnx("export: '%s': not a valid identifier", entry);
        free(name);
        res = 1;
    } else if (remove)
        unexport_var(env, name);
    else if (*word == '\0' && *(word - 1) != '=')
        environment_var_assign(env, name, NULL, true);
    else
        // TODO: wtf is there an expand here
        environment_var_assign(env, name, expand(NULL, word, env, cont), true);
    free(var);
    return res;
}

static void export_print(struct environment *env)
{
    struct htable *vars = env->vars;
    for (size_t i = 0; i < vars->capacity; i++) {
        struct pair *pair = vars->tab[i];
        while (pair) {
            struct variable *var = pair->value;
            if (var->to_export && var->value)
                printf("export %s=\"%s\"\n", pair->key, var->value);
            else if (var->to_export)
                printf("export %s\n", pair->key);
            pair = pair->next;
        }
    }
}

int builtin_export(struct environment *env, struct errcont *cont, int argc, char **argv)
{
    if (!env || !cont)
        warnx("export: missing context elements");
    int res = 0;
    bool print = true;
    bool remove = false;
    for (int i = 1; i < argc; i++) {
        if (!strcmp("-n", argv[i]))
            remove = true;
        else if (strcmp("-p", argv[i])) {
            print = false;
            res |= export_var(env, argv[i], remove, cont);
        }
    }
    if (print)
        export_print(env);
    return res;
}
