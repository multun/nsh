#pragma once

#include "utils/hash_table.h"

struct ast_list;
struct arg_context;

/**
** \brief the runtime shell environment
*/
typedef struct environment
{
    s_htable *vars;
    s_htable *functions;

    // the progname is distinct from the arguments, for example
    // when running functions
    char *progname;
    char **argv;
    int argc;

    // the number of loops we need to break out of
    // this is needed during lbreak exception handling
    size_t break_count;

    // whether the current exception is a continue
    bool break_continue;

    // the loop nesting depth
    size_t depth;

    // current last return code
    int code;
    struct ast_list *ast_list;
} s_env;

/**
** \brief creates an environment from command line arguments
** \param arg_cont the command line arguments to read from
** \return a newly allocated environment
*/
s_env *environment_create(struct arg_context *arg_cont);

/**
** \brief initialize an environment based on the current context
*/
void environment_load(s_env *env);

/**
** \brief free an environment
*/
void environment_free(s_env *env);

/**
** \brief convert the environment to a raw environment variable strings array
*/
char **environment_array(s_env *env);
