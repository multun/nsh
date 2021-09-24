#pragma once

#include <stdbool.h>

#include "utils/hash_table.h"
#include "utils/refcnt.h"
#include "utils/signal_manager.h"
#include "shexec/value.h"
#include "shexec/shopt.h"

struct shast_list;
struct arg_context;

struct shexec_variable
{
    struct hash_head hash;
    bool exported;
    struct sh_value *value;
};


static inline void shexec_variable_destroy(struct shexec_variable *var)
{
    free(hash_head_key(&var->hash));
    sh_value_put(var->value);
}



/**
** \brief the runtime shell environment
*/
struct environment
{
    struct refcnt refcnt;

    struct signal_manager sigman;

    struct hash_table variables;
    struct hash_table functions;
    bool shopts[SHOPT_COUNT];

    // the progname is distinct from the arguments, for example
    // when running functions
    char *progname;
    char **argv;
    int argc;

    // the number of loops we need to break out of
    // this is needed during break / continue exception handling
    size_t break_count;

    // the loop nesting depth
    size_t depth;

    // the function call depth
    size_t call_depth;

    // forked children loose their interactivity
    bool forked;

    // current last return code
    int code;
};

/**
** \brief creates an environment from command line arguments
** \param arg_cont the command line arguments to read from
** \return a newly allocated environment
*/
struct environment *environment_create(struct arg_context *arg_cont);

static inline void environment_get(struct environment *env)
{
    ref_get(&env->refcnt);
}

static inline void environment_put(struct environment *env)
{
    ref_put(&env->refcnt);
}

/**
** \brief convert the environment to a raw environment variable strings array
*/
char **environment_array(struct environment *env);

struct sh_value *environment_var_get(struct environment *env, const char *name);

static inline struct sh_string *environment_var_get_string(struct environment *env, const char *name)
{
    struct sh_value *res = environment_var_get(env, name);
    if (res == NULL)
        return NULL;

    if (!sh_value_is_string(res))
        return NULL;

    return (struct sh_string*)res;
}

static inline const char *environment_var_get_cstring(struct environment *env, const char *name)
{
    struct sh_string *res = environment_var_get_string(env, name);
    if (res == NULL)
        return NULL;
    return sh_string_data(res);
}

void environment_var_assign(struct environment *env, char *name, struct sh_value *value, bool export);

static inline void environment_var_assign_cstring(struct environment *env, char *name, char *value, bool export)
{
    environment_var_assign(env, name, &sh_string_create(value)->base, export);
}

static inline void environment_var_assign_const_cstring(struct environment *env, char *name, char *value, bool export)
{
    environment_var_assign(env, name, &sh_const_string_create(value)->base, export);
}
