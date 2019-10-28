#pragma once

#include "shexec/environment.h"
#include "utils/attr.h"
#include "utils/error.h"
#include "utils/evect.h"

/* the starting expansion buffer size */
#define EXPANSION_DEFAULT_SIZE 100

/**
** \brief expands a string
** \param env the environment used within the expansion
** \param cont the error context to work with
** \return a malloc allocated expanded string
*/
char *expand(struct lineinfo *line_info, char *str, struct environment*env, struct errcont *errcont);

/**
** \brief the expansion context
*/
struct expansion_state {
    struct lineinfo *line_info;
    struct errcont *errcont;
    struct evect vec;
    struct environment *env;
};

static inline void expansion_state_init(struct expansion_state *exp_state,
                                        struct lineinfo *line_info,
                                        struct environment *env)
{
    exp_state->line_info = line_info;
    exp_state->env = env;
    evect_init(&exp_state->vec, EXPANSION_DEFAULT_SIZE);
}

/**
** \brief executes the subshell with buf as input
** \param errcont the error context to work with
** \param buf the the buffed to execute in a subshell
** \param env the environment to expand from
** \param vec the vector to store the result in
*/
void expand_subshell(struct errcont *cont, char *buf, struct environment*env, struct evect *vec);

/**
** \brief expands special single character variables,
**   except positional arguments
** \param res a pointer to target the result with
** \param env the environment to expand from
** \param var the character being considered
*/
char *special_char_lookup(struct environment*env, char var);

/**
** \brief expands to a random integer
** \param res a pointer to target the result with
*/
char *expand_random(void);

/**
** \brief expands the current uid
** \param res a pointer to target the result with
*/
char *expand_uid(void);

char *expand_name(struct environment*env, char *var);

void __noreturn expansion_error(struct expansion_state *exp_state, const char *fmt, ...);
void expansion_warning(struct expansion_state *exp_state, const char *fmt, ...);
