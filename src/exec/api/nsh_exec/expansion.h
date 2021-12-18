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
** \return a malloc allocated expanded string
*/
nsh_err_t expand_nosplit(char **res, struct lineinfo *line_info, const char *str,
                         struct environment *env, int flags) __unused_result;

nsh_err_t expand_wordlist(struct cpvect *res, struct wordlist *wl,
                          struct environment *env, int flags) __unused_result;

nsh_err_t expand_wordlist_callback(struct expansion_callback *callback,
                                   struct wordlist *wl, struct environment *env,
                                   int flags) __unused_result;
