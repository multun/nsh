#pragma once

#include <nsh_exec/environment.h>
#include <nsh_exec/expansion_result.h>
#include <nsh_exec/expansion_callback.h>
#include <nsh_exec/glob.h>
#include <nsh_parse/wordlist.h>
#include <nsh_lex/wlexer.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/attr.h>
#include <nsh_utils/cpvect.h>
#include <nsh_utils/exception.h>
#include <nsh_utils/evect.h>
#include <nsh_lex/variable.h>
#include <nsh_utils/char_bitset.h>

enum expansion_flags
{
    EXP_FLAGS_ASSIGNMENT = 1,
    EXP_FLAGS_PROMPT = 2,
};

/**
** \brief expands a string
** \param env the environment used within the expansion
** \param catcher the exception scope to work with
** \return a malloc allocated expanded string
*/
char *expand_nosplit(struct lineinfo *line_info, const char *str, int flags,
                     struct environment *env, struct exception_catcher *catcher);

nsh_err_t expand_nosplit_compat(char **res, struct lineinfo *line_info, const char *str,
                                int flags, struct environment *env);

struct expansion_state;
struct expansion_result;

void expand(struct expansion_state *exp_state, struct wlexer *wlexer,
            struct exception_catcher *catcher);


void expand_wordlist(struct cpvect *res, struct wordlist *wl, int flags,
                     struct environment *env, struct exception_catcher *catcher);
nsh_err_t expand_wordlist_compat(struct cpvect *res, struct wordlist *wl,
                                 struct environment *env, int flags);

void expand_wordlist_callback(struct expansion_callback *callback, struct wordlist *wl,
                              int flags, struct environment *env,
                              struct exception_catcher *catcher);

nsh_err_t expand_wordlist_callback_compat(struct expansion_callback *callback,
                                          struct wordlist *wl, int flags,
                                          struct environment *env);
