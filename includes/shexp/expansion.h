#pragma once

#include "shexec/environment.h"
#include "utils/error.h"
#include "utils/evect.h"

/**
** \brief expands a string
** \param env the environment used within the expansion
** \param cont the error context to work with
** \return a malloc allocated expanded string
*/
char *expand(char *str, s_env *env, s_errcont *cont);

/**
** \brief expands a subshell into a character vector
** \param errcont the error context to work with
** \param str the original string cursor
** \param env the environment to expand from
** \param vec the vector to store the result in
*/
void expand_subshell(s_errcont *errcont, char **str, s_env *env, s_evect *vec);

/**
** \brief the expansion context
*/
typedef struct exp_ctx
{
    char **str;
    bool *quoted;
} s_exp_ctx;

#define EXPCTX(Str, Quoted)                                                              \
    ((s_exp_ctx){                                                                        \
        .str = (Str),                                                                    \
        .quoted = (Quoted),                                                              \
    })

/**
** \brief expands a backquoted subshell into a character vector
** \param errcont the error context to work with
** \param str the original string cursor
** \param env the environment to expand from
** \param vec the vector to store the result in
** \return whether or not the expansion was performed
*/
bool expand_backquote(s_errcont *errcont, s_exp_ctx ctx, s_env *env, s_evect *vec);

/**
** \brief executes the subshell with buf as input
** \param errcont the error context to work with
** \param buf the the buffed to execute in a subshell
** \param env the environment to expand from
** \param vec the vector to store the result in
*/
void expand_subshell_buffer(s_errcont *errcont, char *buf, s_env *env, s_evect *vec);

/**
** \brief expands an arithmetic expression into a character vector
** \param str the original string cursor
** \param env the environment to expand from
** \param vec the vector to store the result in
** \param errcont the error context to work with
*/
void expand_arth(char **str, s_env *env, s_evect *vec, s_errcont *cont);

/**
** \brief prepare a arithmetic word to be expanded
** \param word the word to prepare for expansion
** \param env the environment used within the expansion
** \param cont the error context to work with
** \return the allocated buffer to give to expand
*/
char *expand_arth_word(char *word, s_env *env, s_errcont *cont);

/**
** \brief expands special single character variables,
**   except positional arguments
** \param res a pointer to target the result with
** \param env the environment to expand from
** \param var the character being considered
*/
bool special_char_lookup(char **res, s_env *env, char var);

/**
** \brief expands to a random integer
** \param res a pointer to target the result with
*/
void expand_random(char **res);

/**
** \brief expands the current uid
** \param res a pointer to target the result with
*/
void expand_uid(char **res);

/**
** \brief tests whether a character should be protected from further expansion
*/
bool expansion_protected_char(char c);
