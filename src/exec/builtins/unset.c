#include <nsh_exec/environment.h>
#include <nsh_lex/variable.h>

#include <err.h>
#include <string.h>


nsh_err_t builtin_unset(struct environment *env, int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        const char *var_name = argv[i];

        /* check that the variable name is valid */
        if (variable_name_check_string(var_name, strlen(var_name)) != 0) {
            warnx("%s: `%s': invalid identifier", argv[0], var_name);
            env->code = 1;
            return NSH_OK;
        }

        struct hashmap_item *variable_head =
            hashmap_find(&env->variables, NULL, var_name);

        /* unsetting already unset variables is fine, is just does nothing */
        if (variable_head == NULL)
            continue;

        /* remove the variable from the hash map */
        hashmap_remove(&env->variables, variable_head);

        /* destroy the variable */
        struct shexec_variable *variable =
            container_of(variable_head, struct shexec_variable, hash);
        shexec_variable_destroy(variable);
        free(variable);
    }

    env->code = 0;
    return NSH_OK;
}
