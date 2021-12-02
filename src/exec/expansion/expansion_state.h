#pragma once

#include <nsh_exec/expansion.h>

/* the starting expansion buffer size */
#define EXPANSION_DEFAULT_SIZE 100


enum expansion_quoting
{
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
struct expansion_state
{
    /* the expansion settings */
    int flags;

    /* a callback to call on each segmented IFS word */
    struct expansion_callback_ctx callback_ctx;

    /* a bitset of whether a char is in the IFS */
    struct char_bitset ifs;

    /* the first char of the ifs, used to join arguments with $* */
    char field_separator_joiner;

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
    * * $doesnotexist has no word, but '' has a word
    */
    bool allow_empty_word;

    /* some globbing specific state */
    struct glob_state glob_state;

    /* the same buffer is re-used during expansion */
    struct variable_name scratch_variable_name;
};

static inline void expansion_state_destroy(struct expansion_state *exp_state)
{
    expansion_result_destroy(&exp_state->result);
    glob_state_destroy(&exp_state->glob_state);
    variable_name_destroy(&exp_state->scratch_variable_name);
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

static inline void expansion_state_reset_data(struct expansion_state *exp_state)
{
    expansion_result_reset(&exp_state->result);
    exp_state->allow_empty_word = false;
}

static inline void expansion_state_init(struct expansion_state *exp_state,
                                        enum expansion_quoting quoting_mode, int flags)
{
    exp_state->flags = flags;
    char_bitset_init(&exp_state->ifs);
    exp_state->field_separator_joiner = ' ';
    exp_state->line_info = NULL;
    exp_state->space_delimited = false;
    expansion_result_init(&exp_state->result, EXPANSION_DEFAULT_SIZE);
    exp_state->quoting_mode = quoting_mode;
    exp_state->allow_empty_word = false;
    glob_state_init(&exp_state->glob_state);
    // reasonable variable name size
    variable_name_init(&exp_state->scratch_variable_name, 16);
}

static inline void expansion_state_set_field_sep(struct expansion_state *exp_state,
                                                 const char *IFS)
{
    if (IFS == NULL)
        return;

    if (IFS[0])
        exp_state->field_separator_joiner = IFS[0];

    for (size_t i = 0; IFS[i]; i++)
        char_bitset_set(&exp_state->ifs, IFS[i], true);
}

static inline struct environment *expansion_state_env(struct expansion_state *exp_state)
{
    return exp_state->callback_ctx.env;
}

__unused_result static inline enum expansion_quoting
expansion_switch_quoting(struct expansion_state *exp_state,
                         enum expansion_quoting new_mode)
{
    enum expansion_quoting old_mode = exp_state->quoting_mode;
    exp_state->quoting_mode = new_mode;
    return old_mode;
}
