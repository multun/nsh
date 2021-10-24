#include <nsh_exec/environment.h>
#include <nsh_lex/variable.h>

#include <err.h>
#include <string.h>


int builtin_unset(struct environment *env, struct ex_scope *ex_scope __unused, int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        const char *var_name = argv[i];

        /* check that the variable name is valid */
        if (variable_name_check_string(var_name, strlen(var_name)) != 0) {
            warnx("%s: `%s': invalid identifier", argv[0], var_name);
            return 1;
        }

        struct hash_head *variable_head = hash_table_find(&env->variables, NULL, var_name);

        /* unsetting already unset variables is fine, is just does nothing */
        if (variable_head == NULL)
            continue;

        /* remove the variable from the hash map */
        hash_table_remove(&env->variables, variable_head);

        /* destroy the variable */
        struct shexec_variable *variable = container_of(variable_head, struct shexec_variable, hash);
        shexec_variable_destroy(variable);
        free(variable);
    }

    return 0;
}
