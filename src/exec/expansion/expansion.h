#pragma once

#include <nsh_exec/expansion.h>
#include <nsh_utils/error.h>

#include "expansion_state.h"


nsh_err_t expansion_error(struct expansion_state *exp_state, const char *fmt,
                          ...) __unused_result;

// flush the buffer even if the word is empty
nsh_err_t expansion_end_word(struct expansion_state *exp_state) __unused_result;

nsh_err_t expansion_push_splitable(struct expansion_state *exp_state,
                                   char c) __unused_result;
nsh_err_t expansion_push_splitable_string(struct expansion_state *exp_state,
                                          const char *str) __unused_result;
void expansion_push_nosplit(struct expansion_state *exp_state, char c);
void expansion_push_nosplit_string(struct expansion_state *exp_state, const char *str);


nsh_err_t expand(struct expansion_state *exp_state, struct wlexer *wlexer);

enum expansion_res
{
    EXPANSION_SUCCESS = 0,
    EXPANSION_FAILURE = 1,
};

/** Returns a negative error or an expansion_res */
int expand_name(struct expansion_state *exp_state, const char *var);
int expand_subshell(struct expansion_state *exp_state, char *subshell_program);
int expand_prompt_escape(struct expansion_state *exp_state, struct wlexer *wlexer,
                         char c);
