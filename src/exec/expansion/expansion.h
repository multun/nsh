#pragma once

#include <nsh_exec/expansion.h>

#include "expansion_state.h"


void __noreturn expansion_error(struct expansion_state *exp_state, const char *fmt, ...);
void expansion_warning(struct expansion_state *exp_state, const char *fmt, ...);

// flush the buffer even if the word is empty
void expansion_end_word(struct expansion_state *exp_state);

void expansion_push_splitable(struct expansion_state *exp_state, char c);
void expansion_push_splitable_string(struct expansion_state *exp_state, const char *str);
void expansion_push_nosplit(struct expansion_state *exp_state, char c);
void expansion_push_nosplit_string(struct expansion_state *exp_state, const char *str);


void expand(struct expansion_state *exp_state, struct wlexer *wlexer,
            struct exception_catcher *catcher);
int expand_name(struct expansion_state *exp_state, const char *var);
void expand_subshell(struct expansion_state *exp_state, char *subshell_program);
enum wlexer_op expand_prompt_escape(struct expansion_state *exp_state,
                                    struct wlexer *wlexer, char c);
