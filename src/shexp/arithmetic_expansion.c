#include "shlex/variable.h"
#include "shexp/expansion.h"
#include "shexp/arithmetic_expansion.h"
#include "utils/mprintf.h"

#include <string.h>
#include <err.h>

static void arith_lexer_advance(struct arith_lexer *alexer)
{
    assert(alexer->lookahead.type != NULL);
    alexer->lookahead.type = NULL;
}

int arith_lexer_peek(struct arith_token *res, struct arith_lexer *alexer)
{
    if (alexer->lookahead.type == NULL) {
        int rc;
        if ((rc = arith_lex(alexer->exp_state, alexer->cs, &alexer->lookahead)))
            return rc;
    }

    *res = alexer->lookahead;
    return 0;
}

static int arith_lexer_pop(struct arith_token *res, struct arith_lexer *alexer)
{
    if (alexer->lookahead.type != NULL) {
        *res = alexer->lookahead;
        alexer->lookahead.type = NULL;
        return 0;
    }

    return arith_lex(alexer->exp_state, alexer->cs, res);
}

static const char *arith_token_repr(struct arith_token *tok)
{
    if (tok->value.type == ARITH_VALUE_STRING)
        return tok->value.data.string;

    return tok->type->name;
}


static int arith_read_name(struct evect *var_name,
                           struct cstream *cs)
{
    int c;
    do {
        c = cstream_peek(cs);
        if (c == EOF)
            break;
        if (!simple_variable_name_check(var_name, c))
            break;
        simple_variable_name_push(var_name, c);
        cstream_pop(cs);
    } while (true);

    if (evect_size(var_name) == 0)
        return 1;

    simple_variable_name_finalize(var_name);
    return 0;
}

static bool arith_starts_name(char c)
{
    return simple_variable_name_check(NULL, c);
}

typedef int arith_t;

arith_t arith_parse_string(const char *str)
{
    return atoi(str);
}

static int arith_string_to_int(struct expansion_state *exp_state, char *string)
{
    char *var_value = expand_name(exp_state->env, string);
    if (var_value == NULL)
        return 0;
    int res = arith_parse_string(var_value);
    free(var_value);
    return res;
}

int arith_value_to_int(struct expansion_state *exp_state, struct arith_value *value)
{
    switch (value->type) {
    case ARITH_VALUE_INTEGER:
        return value->data.integer;
    case ARITH_VALUE_STRING:
        return arith_string_to_int(exp_state, value->data.string);
    case ARITH_VALUE_UNDEFINED:
    default:
        errx(1, "invalid arith_value state");
    }
}

static int arith_lex_name(struct expansion_state *exp_state,
                          struct cstream *cs,
                          struct arith_value *res)
{
    // read the variable name
    struct evect var_name;
    evect_init(&var_name, 16);
    if (arith_read_name(&var_name, cs) != 0) {
        evect_destroy(&var_name);
        expansion_warning(exp_state, "lonely dollar in expansion");
        return 1;
    }

    res->type = ARITH_VALUE_STRING;
    res->data.string = evect_data(&var_name);
    return 0;
}

// this override is perfectly intentionnal :)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
static char operator_start_map[] =
{
#define X(NulPrio, LeftPrio, Name, TokenType, Op, FirstChar, ...) [FirstChar] = 1,
#include "operators.defs"
#undef X
};
#pragma GCC diagnostic pop

bool arith_starts_operator(int c)
{
    int op_map_size = sizeof(operator_start_map);
    if (c <= 0 || c >= op_map_size)
        return false;
    return operator_start_map[c];
}

static bool arith_breaks_expression(char c)
{
    return isspace(c) || arith_starts_operator(c);
}


// the field is zero if the char doesn't map to anything, 1 + value otherwise
static char arith_number_map[127] =
{
    ['0'] = 1,
    ['1'] = 2,
    ['2'] = 3,
    ['3'] = 4,
    ['4'] = 5,
    ['5'] = 6,
    ['6'] = 7,
    ['7'] = 8,
    ['8'] = 9,
    ['9'] = 10,
    ['A'] = 11,
    ['B'] = 12,
    ['C'] = 13,
    ['D'] = 14,
    ['E'] = 15,
    ['F'] = 16,
    ['a'] = 11,
    ['b'] = 12,
    ['c'] = 13,
    ['d'] = 14,
    ['e'] = 15,
    ['f'] = 16,
};

static int arith_parse_digit(int c)
{
    int max_ascii = sizeof(arith_number_map) / sizeof(arith_number_map[0]);
    if (c <= 0 || c >= max_ascii)
        return -1;

    return arith_number_map[c] - 1;
}

static int arith_lex_number_base(struct cstream *cs)
{
    int c = cstream_peek(cs);
    assert(isdigit(c));
    if (c != '0')
        return 10;

    cstream_pop(cs);

    c = cstream_peek(cs);
    if (c != 'x' && c != 'X')
        return 8;

    cstream_pop(cs);
    return 16;
}

static int arith_lex_number(struct expansion_state *exp_state, struct cstream *cs,
                            struct arith_value *res)
{
    int base = arith_lex_number_base(cs);
    res->type = ARITH_VALUE_INTEGER;
    res->data.integer = 0;
    while (true) {
        int c = cstream_peek(cs);
        if (c == EOF || arith_breaks_expression(c))
            break;

        int new_digit = arith_parse_digit(c);
        if (new_digit == -1 || new_digit >= base) {
            expansion_warning(exp_state, "%s isn't a valid base %d digit", c, base);
            return 1;
        }

        res->data.integer = res->data.integer * base + new_digit;
        cstream_pop(cs);
    }
    return 0;
}

#define DEFINE_INFIX(Name, Op)                                                           \
    static int Name(struct arith_value *left, struct arith_token *self,                  \
                    struct arith_lexer *alexer)                                          \
    {                                                                                    \
        int rc;                                                                          \
        int i_left = arith_value_to_int(alexer->exp_state, left);                        \
        arith_value_destroy(left);                                                       \
        struct arith_value right;                                                        \
        if ((rc = arith_parse(&right, alexer, self->type->left_priority)))               \
            return rc;                                                                   \
        int i_right = arith_value_to_int(alexer->exp_state, &right);                     \
        arith_value_destroy(&right);                                                     \
        *left = ARITH_VALUE_INT(i_left Op i_right);                                      \
        return 0;                                                                        \
    }

#define DEFINE_UN(Name, Op)                                                              \
    static int Name(struct arith_value *res, struct arith_token *self,                   \
                    struct arith_lexer *alexer)                                          \
    {                                                                                    \
        int rc;                                                                          \
        struct arith_value right;                                                        \
        if ((rc = arith_parse(&right, alexer, self->type->nul_priority)))                \
            return rc;                                                                   \
        int i_right = arith_value_to_int(alexer->exp_state, &right);                     \
        arith_value_destroy(&right);                                                     \
        *res = ARITH_VALUE_INT(Op i_right);                                              \
        return 0;                                                                        \
    }

// TODO: factor the template to reduce code size
#define DEFINE_ASSIGN_OP(Name, Op)                                                       \
    static int Name(struct arith_value *left, struct arith_token *self __unused,         \
                    struct arith_lexer *alexer)                                          \
    {                                                                                    \
        int rc;                                                                          \
                                                                                         \
        if (left->type != ARITH_VALUE_STRING) {                                          \
            warnx("expected a name as left operand");                                    \
            return 1;                                                                    \
        }                                                                                \
                                                                                         \
        struct arith_value right;                                                        \
        if ((rc = arith_parse(&right, alexer, 0))) {                                     \
            arith_value_destroy(left);                                                   \
            return rc;                                                                   \
        }                                                                                \
                                                                                         \
        int right_int = arith_value_to_int(alexer->exp_state, &right);                   \
        arith_value_destroy(&right);                                                     \
                                                                                         \
        int var_int = arith_string_to_int(alexer->exp_state, left->data.string);         \
        var_int Op right_int;                                                            \
        char *new_value = mprintf("%d", var_int);                                        \
        environment_var_assign(alexer->exp_state->env, left->data.string, new_value,     \
                               false);                                                   \
        /* don't free the key string, as it is used in the hash table */                 \
        *left = ARITH_VALUE_INT(var_int);                                                \
        return 0;                                                                        \
    }

#define TOKEN_OPERATOR_ASSIGN_OP(NulPrio, LeftPrio, Name, Op, ...)                       \
    DEFINE_ASSIGN_OP(arith_##Name##_left, Op)                                            \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

#define TOKEN_OPERATOR_INFIX(NulPrio, LeftPrio, Name, Op, ...)                           \
    DEFINE_INFIX(arith_##Name##_left, Op)                                                \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

#define TOKEN_OPERATOR_PREFIX(NulPrio, LeftPrio, Name, Op, ...)                          \
    DEFINE_UN(arith_##Name##_nul, Op)                                                    \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_nul = arith_##Name##_nul,                                                \
        .nul_priority = NulPrio,                                                         \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

#define TOKEN_OPERATOR_INFIX_PREFIX(NulPrio, LeftPrio, Name, Op, ...)                    \
    DEFINE_INFIX(arith_##Name##_left, Op)                                                \
    DEFINE_UN(arith_##Name##_nul, Op)                                                    \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .handle_nul = arith_##Name##_nul,                                                \
        .left_priority = LeftPrio,                                                       \
        .nul_priority = NulPrio,                                                         \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

extern struct arith_token_type arith_type_rparen;

static int arith_lparen_nul(struct arith_value *res, struct arith_token *self __unused,
                            struct arith_lexer *alexer)
{
    int rc;
    if ((rc = arith_parse(res, alexer, 0)))
        return rc;

    struct arith_token next_token;
    if ((rc = arith_lexer_peek(&next_token, alexer)))
        return rc;

    if (next_token.type != &arith_type_rparen) {
        arith_value_destroy(res);
        // TODO: token detail
        expansion_warning(alexer->exp_state, "unexpected token after lparen expression");
        return 1;
    }

    arith_lexer_advance(alexer);
    return 0;
}

#define TOKEN_OPERATOR_GROUP(NulPrio, LeftPrio, Name, Op, ...)                           \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_nul = arith_lparen_nul,                                                  \
        .nul_priority = NulPrio,                                                         \
        .name = "(",                                                                     \
    };

#define TOKEN_OPERATOR_NOP(NulPrio, LeftPrio, Name, Op, ...)                             \
    struct arith_token_type arith_type_##Name = {                                        \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

static int arith_equal_left(struct arith_value *left, struct arith_token *self __unused,
                            struct arith_lexer *alexer)
{
    int rc;

    if (left->type != ARITH_VALUE_STRING) {
        warnx("expected a name as left operand");
        return 1;
    }

    struct arith_value right;
    if ((rc = arith_parse(&right, alexer, 0))) {
        arith_value_destroy(left);
        return rc;
    }

    int res = arith_value_to_int(alexer->exp_state, &right);
    arith_value_destroy(&right);
    char *new_value = mprintf("%d", res);
    environment_var_assign(alexer->exp_state->env, left->data.string, new_value, false);
    // don't free the key string, as it is used in the hash table
    *left = ARITH_VALUE_INT(res);
    return 0;
}

#define TOKEN_OPERATOR_ASSIGN(NulPrio, LeftPrio, Name, Op, ...)                          \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_equal_left,                                                 \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

#define TOKEN_OPERATOR_TERNARY TOKEN_OPERATOR_NOP
#define TOKEN_OPERATOR_PREFIX_POSTFIX TOKEN_OPERATOR_NOP

#define X(NulPrio, LeftPrio, Name, TokenType, Op, ...)   \
    TokenType(NulPrio, LeftPrio, Name, Op, __VA_ARGS__)
#include "operators.defs"
#undef X

#define OP_TYPE(Name) arith_type_ ## Name

struct arith_operator {
    char *value;
    size_t op_len;
    struct arith_token_type *type;
};
static struct arith_operator arith_operators[] =
{
#define X(NulPrio, LeftPrio, Name, TokenType, Op, ...)                             \
    {                                                                                    \
        .value = (char[]){__VA_ARGS__}, .op_len = sizeof((char[]){__VA_ARGS__}) - 1,     \
        .type = &OP_TYPE(Name)                                                           \
    },
#include "operators.defs"
#undef X
};

static const struct arith_operator *find_operator(const char *buf, size_t size, char next_ch)
{
    // TODO: handle null bytes
    for (size_t i = 0; i < sizeof(arith_operators)/sizeof(arith_operators[0]); i++) {
        struct arith_operator *cur_operator = &arith_operators[i];

        // skip operators shorter than the current buffer size
        if (cur_operator->op_len <= size)
            continue;

        if (strncmp(buf, cur_operator->value, size) == 0
            && cur_operator->value[size] == next_ch)
            return cur_operator;
    }

    return NULL;
}

static int arith_lex_operator(struct cstream *cs,
                              struct arith_token *res)
{
    // TODO: binary search
    const char *data = NULL;
    const struct arith_operator *cur_operator = NULL;
    for (size_t size = 0;; size++) {
        int c = cstream_peek(cs);
        if (c == EOF)
            break;

        const struct arith_operator *better_operator =
            find_operator(data, size, c);
        if (!better_operator)
            break;

        cur_operator = better_operator;
        data = better_operator->value;
        cstream_pop(cs);
    }
    assert(cur_operator != NULL);
    res->type = cur_operator->type;
    res->value = ARITH_VALUE_UND;
    return 0;
}

static int arith_lexer_nul(struct arith_value *res, struct arith_lexer *alexer,
                           struct arith_token *atoken)
{
    f_arith_handler_nul handler = atoken->type->handle_nul;
    if (handler != NULL)
        return handler(res, atoken, alexer);

    expansion_warning(alexer->exp_state,
                      "%s can't start an "
                      "arithmetic expression",
                      arith_token_repr(atoken));
    arith_token_destroy(atoken);
    return 1;
}

static int arith_lexer_left(struct arith_value *res, struct arith_lexer *alexer,
                            struct arith_token *atoken)
{
    f_arith_handler_left handler = atoken->type->handle_left;
    if (handler != NULL)
        return handler(res, atoken, alexer);

    expansion_warning(alexer->exp_state, "%s isn't an arithmetic operator",
                      arith_token_repr(atoken));
    arith_token_destroy(atoken);
    return 1;
}

static int arith_nul_copy_value(
    struct arith_value *res,
    struct arith_token *self,
    struct arith_lexer *alexer __unused) {
    *res = self->value;
    return 0;
}

static struct arith_token_type arith_integer_type = {
    .handle_nul = arith_nul_copy_value,
    .left_priority = 0,
    .name = "integer litteral",
};

static struct arith_token_type arith_identifier_type = {
    .handle_nul = arith_nul_copy_value,
    .left_priority = 0,
    .name = "identifier",
};

struct arith_token_type arith_type_eof = {
    .left_priority = 0,
    .name = "EOF",
};

int arith_lex(struct expansion_state *exp_state, struct cstream *cs,
              struct arith_token *res)
{
    // skip spaces
    int c = cstream_peek(cs);
    for (; isspace(c); (c = cstream_peek(cs)))
        cstream_pop(cs);

    if (c == EOF) {
        res->type = &arith_type_eof;
        res->value = ARITH_VALUE_UND;
        return 0;
    }

    if (arith_starts_name(c)) {
        res->type = &arith_identifier_type;
        return arith_lex_name(exp_state, cs, &res->value);
    }
    if (isdigit(c)) {
        res->type = &arith_integer_type;
        return arith_lex_number(exp_state, cs, &res->value);
    }
    if (arith_starts_operator(c)) {
        return arith_lex_operator(cs, res);
    }
    expansion_warning(exp_state, "unknown character: %c", c);
    return 1;
}

int arith_parse(struct arith_value *res, struct arith_lexer *alexer, int parent_priority)
{
    int rc;

    // lex the first token, and call the nul handler
    struct arith_token atoken;
    if ((rc = arith_lexer_pop(&atoken, alexer)))
        return rc;

    if ((rc = arith_lexer_nul(res, alexer, &atoken)))
        return rc;

    while (true) {
        struct arith_token anext;
        if ((rc = arith_lexer_peek(&anext, alexer))) {
            arith_value_destroy(res);
            return rc;
        }

        if (anext.type->left_priority <= parent_priority)
            break;

        arith_lexer_advance(alexer);
        if (arith_lexer_left(res, alexer, &anext))
            return rc;
    }

    return 0;
}
