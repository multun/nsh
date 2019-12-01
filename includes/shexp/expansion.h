#pragma once

#include "shexec/environment.h"
#include "shexp/expansion_result.h"
#include "shexp/expansion_callback.h"
#include "shexp/glob.h"
#include "shparse/wordlist.h"
#include "shwlex/wlexer.h"
#include "utils/attr.h"
#include "utils/attr.h"
#include "utils/cpvect.h"
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
char *expand_nosplit(struct lineinfo *line_info, char *str, struct environment *env, struct errcont *errcont);

void expand_wordlist(struct cpvect *res, struct wordlist *wl, struct environment *env, struct errcont *errcont);

struct expansion_state;
struct expansion_result;

void expand(struct expansion_state *exp_state,
            struct wlexer *wlexer,
            struct errcont *errcont);


void expand_wordlist(struct cpvect *res, struct wordlist *wl, struct environment *env, struct errcont *errcont);

void expand_wordlist_callback(struct expansion_callback *callback, struct wordlist *wl, struct environment *env, struct errcont *errcont);

enum expansion_quoting {
    // split on IFS
    EXPANSION_QUOTING_UNQUOTED = 0,
    // don't split on IFS, but split on $@
    EXPANSION_QUOTING_QUOTED,
    // never split. it's needed for $@
    EXPANSION_QUOTING_NOSPLIT,
};

/**
** \brief the expansion context
*/
struct expansion_state {
    /* a callback to call on each segmented IFS word */
    struct expansion_callback_ctx callback_ctx;

    const char *IFS;
    struct lineinfo *line_info;

    struct expansion_result result;

    /* are we in an unquoted section */
    enum expansion_quoting quoting_mode;

    /**
    ** Whether the last word was delimited by an IFS space.
    ** When a valid separator is seen, the callback is immediatly called.
    */
    bool space_delimited;

    /**
    ** quoting causes empty word to be allowed:
    ** $doesnotexist has no word, but '' has a word
    */
    bool allow_empty_word;

    /* some globbing specific state */
    struct glob_state glob_state;
};

static inline struct errcont *expansion_state_errcont(struct expansion_state *exp_state)
{
    return exp_state->callback_ctx.errcont;
}

static inline struct environment *expansion_state_env(struct expansion_state *exp_state)
{
    return exp_state->callback_ctx.env;
}

static inline void expansion_state_set_errcont(struct expansion_state *exp_state,
                                               struct errcont *cont)
{
    exp_state->callback_ctx.errcont = cont;
}

static inline void expansion_state_destroy(struct expansion_state *exp_state)
{
    expansion_result_destroy(&exp_state->result);
    glob_state_destroy(&exp_state->glob_state);
}

static inline bool expansion_has_content(struct expansion_state *exp_state)
{
    return expansion_result_size(&exp_state->result) != 0 || exp_state->allow_empty_word;
}

static inline bool expansion_is_unquoted(struct expansion_state *exp_state)
{
    return exp_state->quoting_mode == EXPANSION_QUOTING_UNQUOTED;
}

static inline bool expansion_is_quoted(struct expansion_state *exp_state)
{
    return !expansion_is_unquoted(exp_state);
}

__unused_result static inline enum expansion_quoting expansion_switch_quoting(
    struct expansion_state *exp_state, enum expansion_quoting new_mode)
{
    enum expansion_quoting old_mode = exp_state->quoting_mode;
    exp_state->quoting_mode = new_mode;
    return old_mode;
}

// flush the buffer even if the word is empty
void expansion_end_word(struct expansion_state *exp_state);

static inline void expansion_end_section(struct expansion_state *exp_state)
{
    struct expansion_result *result = &exp_state->result;
    size_t result_size = expansion_result_size(result);
    // if there's no section to end, give up
    if (result_size == 0)
        return;

    size_t last_i = result_size - 1;
    // if the last char isn't unquoted, it can't end an unquoted section
    if (!expansion_result_getflag(result, last_i, EXPANSION_UNQUOTED))
        return;

    expansion_result_setflag(result, last_i, EXPANSION_END_UNQUOTED);
}

void expansion_push(struct expansion_state *exp_state, char c);
void expansion_push_string(struct expansion_state *exp_state, const char *str);

static inline void expansion_state_reset_data(struct expansion_state *exp_state)
{
    expansion_result_reset(&exp_state->result);
    exp_state->allow_empty_word = false;
}

static inline void expansion_state_init(struct expansion_state *exp_state,
                                        enum expansion_quoting quoting_mode)
{
    exp_state->IFS = NULL;
    exp_state->line_info = NULL;
    exp_state->space_delimited = false;
    expansion_result_init(&exp_state->result, EXPANSION_DEFAULT_SIZE);
    exp_state->quoting_mode = quoting_mode;
    exp_state->allow_empty_word = false;
    glob_state_init(&exp_state->glob_state);
}

/**
** \brief executes the subshell with buf as input
** \param exp_state the expansion state
*/
void expand_subshell(struct expansion_state *exp_state, char *buf);

int expand_name(struct expansion_state *exp_state, char *var);

void __noreturn expansion_error(struct expansion_state *exp_state, const char *fmt, ...);
void expansion_warning(struct expansion_state *exp_state, const char *fmt, ...);
