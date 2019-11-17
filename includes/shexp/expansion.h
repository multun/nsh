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

// each output byte has a combination of these two flags:
// it can start an unquoted section, and / or stop one.
// beware, a char can stop a section even though no section
// was opened.
enum expansion_meta {
    EXPANSION_UNQUOTED = 1,
    EXPANSION_END_UNQUOTED = 2,
};

/**
** \brief the expansion context
*/
struct expansion_state {
    const char *IFS;
    struct lineinfo *line_info;
    struct errcont *errcont;
    struct evect result;

    /* each output byte has a corresponding metadata byte */
    struct evect result_meta;

    /* are we in an unquoted section */
    bool unquoted;

    /**
    ** quoting causes empty word to be allowed:
    ** $doesnotexist has no word, but '' has a word
    */
    bool allow_empty_word;

    struct environment *env;

    /* a callback to call on each segmented IFS word */
    expansion_callback_f callback;
    void *callback_data;
};

static inline void expansion_end_section(struct expansion_state *exp_state)
{
    struct evect *meta = &exp_state->result_meta;
    // if there's no section to end, give up
    if (evect_size(meta) == 0)
        return;

    char *last_char = &evect_data(meta)[evect_size(meta) - 1];
    // if the last char isn't unquoted, it can't end an unquoted section
    if (!(*last_char & EXPANSION_UNQUOTED))
        return;

    *last_char |= EXPANSION_END_UNQUOTED;
}

void expansion_push(struct expansion_state *exp_state, char c);

static inline void expansion_state_init(struct expansion_state *exp_state,
                                        struct lineinfo *line_info,
                                        struct environment *env)
{
    exp_state->IFS = NULL;
    exp_state->line_info = line_info;
    exp_state->env = env;
    exp_state->unquoted = true;
    exp_state->allow_empty_word = false;
    exp_state->callback = NULL;
    evect_init(&exp_state->result, EXPANSION_DEFAULT_SIZE);
    evect_init(&exp_state->result_meta, EXPANSION_DEFAULT_SIZE);
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
