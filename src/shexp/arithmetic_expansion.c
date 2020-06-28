#include "shlex/variable.h"
#include "shexp/expansion.h"
#include "shexp/arithmetic_expansion.h"
#include "utils/mprintf.h"
#include "utils/parsing.h"

#include <string.h>
#include <err.h>

static void arith_lexer_advance(struct arith_lexer *alexer)
{
    assert(alexer->lookahead.type != NULL);
    alexer->lookahead.type = NULL;
}

enum arith_status arith_lexer_peek(struct arith_token *res, struct arith_lexer *alexer)
{
    if (alexer->lookahead.type == NULL) {
        enum arith_status rc;
        if ((rc = arith_lex(alexer->exp_state, alexer->cs, &alexer->lookahead)))
            return rc;
    }

    *res = alexer->lookahead;
    return ARITH_OK;
}

static enum arith_status arith_lexer_pop(struct arith_token *res,
                                         struct arith_lexer *alexer)
{
    if (alexer->lookahead.type != NULL) {
        *res = alexer->lookahead;
        alexer->lookahead.type = NULL;
        return ARITH_OK;
    }

    return arith_lex(alexer->exp_state, alexer->cs, res);
}

static const char *arith_token_repr(struct arith_token *tok)
{
    if (tok->value.type == ARITH_VALUE_STRING)
        return tok->value.data.string;

    return tok->type->name;
}

static enum arith_status arith_read_name(struct evect *var_name, struct cstream *cs)
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
        return ARITH_SYNTAX_ERROR;

    simple_variable_name_finalize(var_name);
    return ARITH_OK;
}

static bool arith_starts_name(char c)
{
    return simple_variable_name_check(NULL, c);
}

typedef int arith_t;

static arith_t arith_parse_string(const char *str)
{
    return atoi(str);
}

static enum arith_status arith_string_to_int(struct expansion_state *exp_state,
                                             char *string)
{
    size_t cur_size = expansion_result_size(&exp_state->result);
    if (expand_name(exp_state, string) != 0) {
        expansion_result_cut(&exp_state->result, cur_size);
        return 0;
    }

    evect_push(&exp_state->result.string, '\0');
    int res = arith_parse_string(expansion_result_data(&exp_state->result) + cur_size);
    expansion_result_cut(&exp_state->result, cur_size);
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

static enum arith_status arith_lex_name(struct expansion_state *exp_state,
                                        struct cstream *cs, struct arith_value *res)
{
    // read the variable name
    struct evect var_name;
    evect_init(&var_name, 16);
    if (arith_read_name(&var_name, cs) != 0) {
        evect_destroy(&var_name);
        expansion_warning(exp_state, "lonely dollar in expansion");
        return ARITH_SYNTAX_ERROR;
    }

    res->type = ARITH_VALUE_STRING;
    res->data.string = evect_data(&var_name);
    return ARITH_OK;
}

// this override is perfectly intentionnal :)
#pragma GCC diagnostic push
#ifdef __clang__
#    pragma GCC diagnostic ignored "-Winitializer-overrides"
#else
#    pragma GCC diagnostic ignored "-Woverride-init"
#endif
static char operator_start_map[] = {
#define X(NulPrio, LeftPrio, Name, TokenType, Op, FirstChar, ...) [FirstChar] = 1,
#include "operators.defs"
#undef X
};
#pragma GCC diagnostic pop

static bool arith_starts_operator(int c)
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

static enum arith_status arith_lex_number(struct expansion_state *exp_state,
                                          struct cstream *cs, struct arith_value *res)
{
    int base = arith_lex_number_base(cs);
    res->type = ARITH_VALUE_INTEGER;
    res->data.integer = 0;
    while (true) {
        int c = cstream_peek(cs);
        if (c == EOF || arith_breaks_expression(c))
            break;

        int new_digit = parse_digit(c);
        if (new_digit == -1 || new_digit >= base) {
            expansion_warning(exp_state, "'%c' (0x%x) isn't a valid base %d digit", c, c,
                              base);
            return ARITH_SYNTAX_ERROR;
        }

        res->data.integer = res->data.integer * base + new_digit;
        cstream_pop(cs);
    }
    return ARITH_OK;
}

// predefine operator types
#define X(NulPrio, LeftPrio, Name, TokenType, Op, ...)                                   \
    extern struct arith_token_type arith_type_##Name;
#include "operators.defs"
#undef X

/* infix operators */

#define DEFINE_INFIX(Name, Op)                                                           \
    static enum arith_status Name(struct arith_value *left, struct arith_token *self,    \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        enum arith_status rc;                                                            \
        int i_left = arith_value_to_int(alexer->exp_state, left);                        \
        arith_value_destroy(left);                                                       \
        struct arith_value right;                                                        \
        if ((rc = arith_parse(&right, alexer, self->type->left_priority)))               \
            return rc;                                                                   \
        int i_right = arith_value_to_int(alexer->exp_state, &right);                     \
        arith_value_destroy(&right);                                                     \
        *left = ARITH_VALUE_INT(i_left Op i_right);                                      \
        return ARITH_OK;                                                                 \
    }

#define TOKEN_OPERATOR_INFIX(NulPrio, LeftPrio, Name, Op, ...)                           \
    DEFINE_INFIX(arith_##Name##_left, Op)                                                \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* unary operators */

#define DEFINE_UN(Name, Op)                                                              \
    static enum arith_status Name(struct arith_value *res, struct arith_token *self,     \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        enum arith_status rc;                                                            \
        struct arith_value right;                                                        \
        if ((rc = arith_parse(&right, alexer, self->type->nul_priority)))                \
            return rc;                                                                   \
        int i_right = arith_value_to_int(alexer->exp_state, &right);                     \
        arith_value_destroy(&right);                                                     \
        *res = ARITH_VALUE_INT(Op i_right);                                              \
        return ARITH_OK;                                                                 \
    }

#define TOKEN_OPERATOR_PREFIX(NulPrio, LeftPrio, Name, Op, ...)                          \
    DEFINE_UN(arith_##Name##_nul, Op)                                                    \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_nul = arith_##Name##_nul,                                                \
        .nul_priority = NulPrio,                                                         \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* += and friends */

#define DEFINE_ASSIGN_OP(Name, Op)                                                       \
    static enum arith_status Name(struct arith_value *left,                              \
                                  struct arith_token *self __unused,                     \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        enum arith_status rc;                                                            \
                                                                                         \
        if (left->type != ARITH_VALUE_STRING) {                                          \
            warnx("expected a name as left operand");                                    \
            return ARITH_SYNTAX_ERROR;                                                   \
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
        environment_var_assign_cstring(expansion_state_env(alexer->exp_state),           \
                                       left->data.string, new_value, false);             \
        /* don't free the key string, as it is used in the hash table */                 \
        *left = ARITH_VALUE_INT(var_int);                                                \
        return ARITH_OK;                                                                 \
    }

#define TOKEN_OPERATOR_ASSIGN_OP(NulPrio, LeftPrio, Name, Op, ...)                       \
    DEFINE_ASSIGN_OP(arith_##Name##_left, Op)                                            \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* /= and %=, which has a check for div by 0 */

#define DEFINE_DIV_ASSIGN_OP(Name, Op)                                                   \
    static enum arith_status Name(struct arith_value *left,                              \
                                  struct arith_token *self __unused,                     \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        enum arith_status rc;                                                            \
                                                                                         \
        if (left->type != ARITH_VALUE_STRING) {                                          \
            warnx("expected a name as left operand");                                    \
            return ARITH_SYNTAX_ERROR;                                                   \
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
        if (right_int == 0) {                                                            \
            expansion_warning(alexer->exp_state, "division by 0");                       \
            return ARITH_RUNTIME_ERROR;                                                  \
        }                                                                                \
                                                                                         \
        int var_int = arith_string_to_int(alexer->exp_state, left->data.string);         \
        var_int Op right_int;                                                            \
        char *new_value = mprintf("%d", var_int);                                        \
        environment_var_assign_cstring(expansion_state_env(alexer->exp_state),           \
                                       left->data.string, new_value, false);             \
        /* don't free the key string, as it is used in the hash table */                 \
        *left = ARITH_VALUE_INT(var_int);                                                \
        return ARITH_OK;                                                                 \
    }

#define TOKEN_OPERATOR_DIV_EQUAL(NulPrio, LeftPrio, Name, Op, ...)                       \
    DEFINE_DIV_ASSIGN_OP(arith_##Name##_left, Op)                                        \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* / and % operators */

#define DEFINE_DIV(Name, Op)                                                             \
    static enum arith_status Name(struct arith_value *left, struct arith_token *self,    \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        enum arith_status rc;                                                            \
        int i_left = arith_value_to_int(alexer->exp_state, left);                        \
        arith_value_destroy(left);                                                       \
        struct arith_value right;                                                        \
        if ((rc = arith_parse(&right, alexer, self->type->left_priority)))               \
            return rc;                                                                   \
        int i_right = arith_value_to_int(alexer->exp_state, &right);                     \
        arith_value_destroy(&right);                                                     \
                                                                                         \
        if (i_right == 0) {                                                              \
            expansion_warning(alexer->exp_state, "division by 0");                       \
            return ARITH_RUNTIME_ERROR;                                                  \
        }                                                                                \
                                                                                         \
        *left = ARITH_VALUE_INT(i_left Op i_right);                                      \
        return ARITH_OK;                                                                 \
    }

#define TOKEN_OPERATOR_DIV(NulPrio, LeftPrio, Name, Op, ...)                             \
    DEFINE_DIV(arith_##Name##_left, Op)                                                  \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_##Name##_left,                                              \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* ( ) grouping operator */

static enum arith_status arith_lparen_nul(struct arith_value *res,
                                          struct arith_token *self __unused,
                                          struct arith_lexer *alexer)
{
    enum arith_status rc;
    if ((rc = arith_parse(res, alexer, 0)))
        return rc;

    struct arith_token next_token;
    if ((rc = arith_lexer_peek(&next_token, alexer)))
        return rc;

    if (next_token.type != &arith_type_rparen) {
        arith_value_destroy(res);
        // TODO: token detail
        expansion_warning(alexer->exp_state, "unexpected token after lparen expression");
        return ARITH_SYNTAX_ERROR;
    }

    arith_lexer_advance(alexer);
    return ARITH_OK;
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

/* = operator */

static enum arith_status arith_equal_left(struct arith_value *left,
                                          struct arith_token *self __unused,
                                          struct arith_lexer *alexer)
{
    enum arith_status rc;

    if (left->type != ARITH_VALUE_STRING) {
        expansion_warning(alexer->exp_state, "expected a name as left operand");
        return ARITH_SYNTAX_ERROR;
    }

    struct arith_value right;
    if ((rc = arith_parse(&right, alexer, 0))) {
        arith_value_destroy(left);
        return rc;
    }

    int res = arith_value_to_int(alexer->exp_state, &right);
    arith_value_destroy(&right);
    char *new_value = mprintf("%d", res);
    environment_var_assign_cstring(expansion_state_env(alexer->exp_state),
                                   left->data.string, new_value, false);
    // don't free the key string, as it is used in the hash table
    *left = ARITH_VALUE_INT(res);
    return ARITH_OK;
}

#define TOKEN_OPERATOR_ASSIGN(NulPrio, LeftPrio, Name, Op, ...)                          \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_equal_left,                                                 \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* ternary operator */

static enum arith_status arith_ternary_left(struct arith_value *left,
                                            struct arith_token *self __unused,
                                            struct arith_lexer *alexer)
{
    enum arith_status rc;
    int condition = arith_value_to_int(alexer->exp_state, left);

    // parse the true branch
    struct arith_value true_branch;
    if ((rc = arith_parse(&true_branch, alexer, 0)))
        return rc;
    int true_value = arith_value_to_int(alexer->exp_state, &true_branch);
    arith_value_destroy(&true_branch);

    // find the colon separator
    struct arith_token colon_token;
    if ((rc = arith_lexer_pop(&colon_token, alexer)))
        return rc;
    if (colon_token.type != &arith_type_colon) {
        expansion_warning(alexer->exp_state, "expected a colon");
        return ARITH_SYNTAX_ERROR;
    }

    // parse the false branch
    struct arith_value false_branch;
    if ((rc = arith_parse(&false_branch, alexer, 0)))
        return rc;
    int false_value = arith_value_to_int(alexer->exp_state, &false_branch);
    arith_value_destroy(&false_branch);

    *left = ARITH_VALUE_INT((condition ? true_value : false_value));
    return ARITH_OK;
}

#define TOKEN_OPERATOR_TERNARY(NulPrio, LeftPrio, Name, Op, ...)                         \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_left = arith_ternary_left,                                               \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* val++ style operators */

#define DEFINE_POSTFIX_INCRDECR(Name, Op)                                                \
    static enum arith_status Name(struct arith_value *left,                              \
                                  struct arith_token *self __unused,                     \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        if (left->type != ARITH_VALUE_STRING) {                                          \
            warnx("expected a name as left operand");                                    \
            return ARITH_SYNTAX_ERROR;                                                   \
        }                                                                                \
                                                                                         \
        int var_int = arith_string_to_int(alexer->exp_state, left->data.string);         \
        int old_value = var_int;                                                         \
        var_int Op;                                                                      \
        char *new_value = mprintf("%d", var_int);                                        \
        environment_var_assign_cstring(expansion_state_env(alexer->exp_state),           \
                                       left->data.string, new_value, false);             \
        /* don't free the key string, as it is used in the hash table */                 \
        *left = ARITH_VALUE_INT(old_value);                                              \
        return ARITH_OK;                                                                 \
    }

/* ++val style operators */

#define DEFINE_PREFIX_INCRDECR(Name, Op)                                                 \
    static enum arith_status Name(struct arith_value *res,                               \
                                  struct arith_token *self __unused,                     \
                                  struct arith_lexer *alexer)                            \
    {                                                                                    \
        enum arith_status rc;                                                            \
        struct arith_token right;                                                        \
        if ((rc = arith_lexer_pop(&right, alexer)))                                      \
            return rc;                                                                   \
                                                                                         \
        if (right.type != &arith_type_identifier) {                                      \
            warnx("expected a name as right operand");                                   \
            return ARITH_SYNTAX_ERROR;                                                   \
        }                                                                                \
        int var_int = arith_string_to_int(alexer->exp_state, right.value.data.string);   \
        Op var_int;                                                                      \
        char *new_value = mprintf("%d", var_int);                                        \
        environment_var_assign_cstring(expansion_state_env(alexer->exp_state),           \
                                       right.value.data.string, new_value, false);       \
        /* don't free the key string, as it is used in the hash table */                 \
        *res = ARITH_VALUE_INT(var_int);                                                 \
        return ARITH_OK;                                                                 \
    }

#define TOKEN_OPERATOR_PREFIX_POSTFIX(NulPrio, LeftPrio, Name, Op, ...)                  \
    DEFINE_PREFIX_INCRDECR(arith_##Name##_nul, Op)                                       \
    DEFINE_POSTFIX_INCRDECR(arith_##Name##_left, Op)                                     \
    struct arith_token_type arith_type_##Name = {                                        \
        .handle_nul = arith_##Name##_nul,                                                \
        .handle_left = arith_##Name##_left,                                              \
        .nul_priority = NulPrio,                                                         \
        .left_priority = LeftPrio,                                                       \
        .name = (const char[]){__VA_ARGS__},                                             \
    };

/* + and - operators */

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

#define X(NulPrio, LeftPrio, Name, TokenType, Op, ...)                                   \
    TokenType(NulPrio, LeftPrio, Name, Op, __VA_ARGS__)
#include "operators.defs"
#undef X

struct arith_operator
{
    char *value;
    size_t op_len;
    struct arith_token_type *type;
};
static struct arith_operator arith_operators[] = {
#define OP_TYPE(Name) arith_type_##Name
#define X(NulPrio, LeftPrio, Name, TokenType, Op, ...)                                   \
    {.value = (char[]){__VA_ARGS__},                                                     \
     .op_len = sizeof((char[]){__VA_ARGS__}) - 1,                                        \
     .type = &OP_TYPE(Name)},
#include "operators.defs"
#undef X
#undef OP_TYPE
};

static const struct arith_operator *find_operator(const char *buf, size_t size,
                                                  char next_ch)
{
    // TODO: handle null bytes
    for (size_t i = 0; i < sizeof(arith_operators) / sizeof(arith_operators[0]); i++) {
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

static enum arith_status arith_lex_operator(struct cstream *cs, struct arith_token *res)
{
    // TODO: binary search
    const char *data = NULL;
    const struct arith_operator *cur_operator = NULL;
    for (size_t size = 0;; size++) {
        int c = cstream_peek(cs);
        if (c == EOF)
            break;

        const struct arith_operator *better_operator = find_operator(data, size, c);
        if (!better_operator)
            break;

        cur_operator = better_operator;
        data = better_operator->value;
        cstream_pop(cs);
    }
    assert(cur_operator != NULL);
    res->type = cur_operator->type;
    res->value = ARITH_VALUE_UND;
    return ARITH_OK;
}

static enum arith_status arith_lexer_nul(struct arith_value *res,
                                         struct arith_lexer *alexer,
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
    return ARITH_SYNTAX_ERROR;
}

static enum arith_status arith_lexer_left(struct arith_value *res,
                                          struct arith_lexer *alexer,
                                          struct arith_token *atoken)
{
    f_arith_handler_left handler = atoken->type->handle_left;
    if (handler != NULL)
        return handler(res, atoken, alexer);

    expansion_warning(alexer->exp_state, "%s isn't an arithmetic operator",
                      arith_token_repr(atoken));
    arith_token_destroy(atoken);
    return ARITH_SYNTAX_ERROR;
}

static enum arith_status arith_nul_copy_value(struct arith_value *res,
                                              struct arith_token *self,
                                              struct arith_lexer *alexer __unused)
{
    *res = self->value;
    return ARITH_OK;
}

struct arith_token_type arith_type_integer = {
    .handle_nul = arith_nul_copy_value,
    .left_priority = 0,
    .name = "integer litteral",
};

struct arith_token_type arith_type_identifier = {
    .handle_nul = arith_nul_copy_value,
    .left_priority = 0,
    .name = "identifier",
};

struct arith_token_type arith_type_eof = {
    .left_priority = 0,
    .name = "EOF",
};

enum arith_status arith_lex(struct expansion_state *exp_state, struct cstream *cs,
                            struct arith_token *res)
{
    // skip spaces
    int c = cstream_peek(cs);
    for (; isspace(c); (c = cstream_peek(cs)))
        cstream_pop(cs);

    if (c == EOF) {
        res->type = &arith_type_eof;
        res->value = ARITH_VALUE_UND;
        return ARITH_OK;
    }

    if (arith_starts_name(c)) {
        res->type = &arith_type_identifier;
        return arith_lex_name(exp_state, cs, &res->value);
    }
    if (isdigit(c)) {
        res->type = &arith_type_integer;
        return arith_lex_number(exp_state, cs, &res->value);
    }
    if (arith_starts_operator(c)) {
        return arith_lex_operator(cs, res);
    }

    expansion_warning(exp_state, "unknown character: %c", c);
    return ARITH_SYNTAX_ERROR;
}

enum arith_status arith_parse(struct arith_value *res, struct arith_lexer *alexer,
                              int parent_priority)
{
    enum arith_status rc;

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
        if ((rc = arith_lexer_left(res, alexer, &anext)))
            return rc;
    }

    return ARITH_OK;
}
