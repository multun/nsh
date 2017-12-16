#pragma once

#include "shexec/environment.h"
#include "utils/error.h"
#include "utils/evect.h"

typedef struct exp_ctx
{
  char **str;
  bool *quoted;
} s_exp_ctx;


#define EXPCTX(Str, Quoted)                                                  \
((s_exp_ctx)                                                                 \
{                                                                            \
  .str = Str,                                                                \
  .quoted = Quoted,                                                          \
})

char *expand(char *str, s_env *env, s_errcont *cont);
void expand_subshell(s_errcont *errcont, char **str, s_env *env, s_evect *vec);
void expand_arth(char **str, s_env *env, s_evect *vec);

bool special_char_lookup(char **res, s_env *env, char *var);
void expand_random(char **res);
void expand_uid(char **res);

bool expansion_protected_char(char c);
