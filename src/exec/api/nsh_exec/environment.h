#pragma once

#include <stdbool.h>

#include <nsh_utils/hashmap.h>
#include <nsh_utils/refcnt.h>
#include <nsh_utils/error.h>
#include <nsh_utils/signal_manager.h>
#include <nsh_exec/value.h>
#include <nsh_exec/shopt.h>


struct shast_list;
struct cli_options;

struct shexec_variable
{
    struct hashmap_item hash;
    bool exported;
    struct sh_value *value;
};


static inline void shexec_variable_destroy(struct shexec_variable *var)
{
    free(var->hash.key);
    sh_value_put(var->value);
}


struct environment;

/**
** \brief lists all available builtins
*/
#define BUILTINS_APPLY(F)                                                                \
    F(break)                                                                             \
    F(cd)                                                                                \
    F(continue)                                                                          \
    F(echo)                                                                              \
    F(exit)                                                                              \
    F(export)                                                                            \
    F(printf)                                                                            \
    F(shopt)                                                                             \
    F(source)                                                                            \
    F(unset)

#define BUILTINS_DECLARE(Name)                                                           \
    nsh_err_t builtin_##Name(struct environment *env, int argc, char **argv);

typedef nsh_err_t (*f_builtin)(struct environment *env, int argc, char **argv);

BUILTINS_APPLY(BUILTINS_DECLARE)

/**
** \brief searches for a builtin
** \param name the name of the builtin to look for
** \return either a pointer to the builtin, or NULL if none was found
*/
f_builtin find_default_builtin(const char *name);


/**
** \brief the runtime shell environment
*/
struct environment
{
    struct refcnt refcnt;

    struct signal_manager sigman;

    f_builtin (*find_builtin)(const char *name);

    struct hashmap variables;
    struct hashmap functions;
    bool shopts[SHOPT_COUNT];

    // the progname is distinct from the arguments, for example
    // when running functions
    char *progname;
    char **argv;
    int argc;

    // the number of loops we need to break out of
    // this is needed for break / continue
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
struct environment *environment_load(struct cli_options *arg_cont);

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

static inline struct sh_string *environment_var_get_string(struct environment *env,
                                                           const char *name)
{
    struct sh_value *res = environment_var_get(env, name);
    if (res == NULL)
        return NULL;

    if (!sh_value_is_string(res))
        return NULL;

    return (struct sh_string *)res;
}

static inline const char *environment_var_get_cstring(struct environment *env,
                                                      const char *name)
{
    struct sh_string *res = environment_var_get_string(env, name);
    if (res == NULL)
        return NULL;
    return sh_string_data(res);
}

void environment_var_assign(struct environment *env, char *name, struct sh_value *value,
                            bool export);

static inline void environment_var_assign_cstring(struct environment *env, char *name,
                                                  char *value, bool export)
{
    environment_var_assign(env, name, &sh_string_create(value)->base, export);
}

static inline void environment_var_assign_const_cstring(struct environment *env,
                                                        char *name, const char *value,
                                                        bool export)
{
    environment_var_assign(env, name, &sh_const_string_create(value)->base, export);
}
