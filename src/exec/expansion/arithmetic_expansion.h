#pragma once

#include <stdlib.h>
#include <stdbool.h>

#include <nsh_io/cstream.h>
#include "expansion.h"

enum arith_value_type
{
    ARITH_VALUE_UNDEFINED,
    ARITH_VALUE_INTEGER,
    ARITH_VALUE_STRING,
};

struct arith_value
{
    enum arith_value_type type;

    union
    {
        int integer;
        char *string;
    } data;
};

static inline void arith_value_destroy(struct arith_value *val)
{
    if (val->type == ARITH_VALUE_STRING)
        free(val->data.string);
}

struct arith_token_type;
struct arith_token
{
    struct arith_token_type *type;
    struct arith_value value;
};

static inline void arith_token_destroy(struct arith_token *tok)
{
    arith_value_destroy(&tok->value);
}

struct arith_lexer
{
    struct expansion_state *exp_state;
    struct cstream *cs;
    struct arith_token lookahead;
    // disable assignations and subshell execution
    bool pure;
};

#define ARITH_VALUE_UND                                                                  \
    (struct arith_value)                                                                 \
    {                                                                                    \
        .type = ARITH_VALUE_UNDEFINED, .data.integer = 0,                                \
    }

#define ARITH_VALUE_INT(Val)                                                             \
    (struct arith_value)                                                                 \
    {                                                                                    \
        .type = ARITH_VALUE_INTEGER, .data.integer = (Val),                              \
    }

struct arith_token;

typedef nsh_err_t (*f_arith_handler_nul)(struct arith_value *res,
                                         struct arith_token *self,
                                         struct arith_lexer *alexer);
typedef nsh_err_t (*f_arith_handler_left)(struct arith_value *left,
                                          struct arith_token *self,
                                          struct arith_lexer *alexer);

struct arith_token_type
{
    f_arith_handler_nul handle_nul;
    f_arith_handler_left handle_left;
    int nul_priority;
    int left_priority;
    const char *name;
};

nsh_err_t arith_lex(struct expansion_state *exp_state, struct cstream *cs,
                    struct arith_token *res);
nsh_err_t arith_parse(struct arith_value *res, struct arith_lexer *alexer,
                      int parent_priority);
nsh_err_t arith_lexer_peek(struct arith_token *res, struct arith_lexer *alexer);
int arith_value_to_int(struct expansion_state *exp_state, struct arith_value *value);

extern struct arith_token_type arith_type_eof;
extern struct arith_token_type arith_type_identifier;
extern struct arith_token_type arith_type_integer;
