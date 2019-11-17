#pragma once

#include "shexec/environment.h"
#include "shwlex/wlexer.h"
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
char *expand_nosplit(struct lineinfo *line_info, char *str, struct environment*env, struct errcont *errcont);

struct expansion_state;

void expand(struct expansion_state *exp_state,
            struct wlexer *wlexer,
            struct errcont *errcont);

typedef void (*expansion_callback_f)(struct expansion_state *exp_state, void *data);

/**
** \brief the expansion context
*/
struct expansion_state {
    const char *IFS;
    struct lineinfo *line_info;
    struct errcont *errcont;
    struct evect result;
    struct evect result_unquoted;
    bool unquoted;
    struct environment *env;
    expansion_callback_f callback;
    void *callback_data;
};

static inline void expansion_state_init(struct expansion_state *exp_state,
                                        struct lineinfo *line_info,
                                        struct environment *env)
{
    exp_state->IFS = NULL;
    exp_state->line_info = line_info;
    exp_state->env = env;
    exp_state->unquoted = true;
    exp_state->callback = NULL;
    evect_init(&exp_state->result, EXPANSION_DEFAULT_SIZE);
    evect_init(&exp_state->result_unquoted, EXPANSION_DEFAULT_SIZE);
}

void expansion_push(struct expansion_state *exp_state, char c);

static inline void expansion_push_string(struct expansion_state *exp_state, char *str)
{
    for (; *str; str++)
        expansion_push(exp_state, *str);
}

/**
** \brief executes the subshell with buf as input
** \param exp_state the expansion state
*/
void expand_subshell(struct expansion_state *exp_state, char *buf);

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
