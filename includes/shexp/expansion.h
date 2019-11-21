#pragma once

#include "shexec/environment.h"
#include "shparse/wordlist.h"
#include "shwlex/wlexer.h"
#include "utils/attr.h"
#include "utils/cpvect.h"
#include "utils/error.h"
#include "utils/evect.h"
#include "utils/attr.h"


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

void expand(struct expansion_state *exp_state,
            struct wlexer *wlexer,
            struct errcont *errcont);

typedef void (*expansion_callback_f)(struct expansion_state *exp_state, void *data);

struct expansion_callback {
    expansion_callback_f func;
    void *data;
};

void expand_wordlist(struct cpvect *res, struct wordlist *wl, struct environment *env, struct errcont *errcont);

void expand_wordlist_callback(struct expansion_callback *callback, struct wordlist *wl, struct environment *env, struct errcont *errcont);

// each output byte has a combination of these two flags:
// it can start an unquoted section, and / or stop one.
// beware, a char can stop a section even though no section
// was opened.
enum expansion_meta {
    EXPANSION_UNQUOTED = 1,
    EXPANSION_END_UNQUOTED = 2,
};

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
    const char *IFS;
    struct lineinfo *line_info;
    struct errcont *errcont;
    struct evect result;

    /* each output byte has a corresponding metadata byte */
    struct evect result_meta;

    /* are we in an unquoted section */
    enum expansion_quoting quoting_mode;

    /**
    ** quoting causes empty word to be allowed:
    ** $doesnotexist has no word, but '' has a word
    */
    bool allow_empty_word;

    struct environment *env;

    /* a callback to call on each segmented IFS word */
    struct expansion_callback callback;
};

static inline size_t expansion_result_size(struct expansion_state *exp_state)
{
    return evect_size(&exp_state->result);
}

static inline void expansion_result_cut(struct expansion_state *exp_state, size_t i)
{
    assert(i <= exp_state->result.size);
    assert(i <= exp_state->result_meta.size);
    exp_state->result.size = i;
    exp_state->result_meta.size = i;
}

static inline bool expansion_is_unquoted(struct expansion_state *exp_state)
{
    return exp_state->quoting_mode == EXPANSION_QUOTING_UNQUOTED;
}

__unused_result static inline enum expansion_quoting expansion_switch_quoting(
    struct expansion_state *exp_state, enum expansion_quoting new_mode)
{
    enum expansion_quoting old_mode = exp_state->quoting_mode;
    exp_state->quoting_mode = new_mode;
    return old_mode;
}

// flush the buffer to the callback
void expansion_end_word(struct expansion_state *exp_state);

// flush the buffer even if the word is empty
void expansion_force_end_word(struct expansion_state *exp_state);

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
void expansion_push_string(struct expansion_state *exp_state, const char *str);

static inline void expansion_state_reset_data(struct expansion_state *exp_state)
{
    evect_reset(&exp_state->result);
    evect_reset(&exp_state->result_meta);
}

static inline void expansion_state_init(struct expansion_state *exp_state,
                                        struct environment *env,
                                        enum expansion_quoting quoting_mode)
{
    exp_state->IFS = NULL;
    exp_state->line_info = NULL;
    exp_state->env = env;
    exp_state->callback.func = NULL;
    evect_init(&exp_state->result, EXPANSION_DEFAULT_SIZE);
    evect_init(&exp_state->result_meta, EXPANSION_DEFAULT_SIZE);
    exp_state->quoting_mode = quoting_mode;
    exp_state->allow_empty_word = false;
}

/**
** \brief executes the subshell with buf as input
** \param exp_state the expansion state
*/
void expand_subshell(struct expansion_state *exp_state, char *buf);

int expand_name(struct expansion_state *exp_state, char *var);

void __noreturn expansion_error(struct expansion_state *exp_state, const char *fmt, ...);
void expansion_warning(struct expansion_state *exp_state, const char *fmt, ...);
