#pragma once

#include "utils/hash_table.h"
#include "utils/refcnt.h"
#include "utils/signal_manager.h"

struct shast_list;
struct arg_context;

struct shexec_variable
{
    struct hash_head hash;
    bool to_export;
    char *value;
};

/**
** \brief the runtime shell environment
*/
struct environment
{
    struct refcnt refcnt;

    struct signal_manager sigman;

    struct hash_table variables;
    struct hash_table functions;

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

const char *environment_var_get(struct environment *env, const char *name);

void environment_var_assign(struct environment *env, char *name, char *value, bool export);
