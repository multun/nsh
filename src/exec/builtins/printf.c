#include <stdio.h>
#include <string.h>
#include <nsh_exec/environment.h>
#include <nsh_utils/evect.h>
#include <nsh_utils/parsing.h>
#include <nsh_utils/strutils.h>


/*
** https://pubs.opengroup.org/onlinepubs/9699919799/utilities/printf.html
** https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap05.html#tag_05
*/

static int printf_help(char *argv[])
{
    fprintf(stderr, "usage: %s [-v var] format [arguments]\n", argv[0]);
    return 1;
}

static void write_char(struct evect *dest, char c)
{
    /* if the destination is NULL, we're not writing to a variable */
    if (dest == NULL) {
        putchar(c);
        return;
    }

    /* write to the result buffer when using -v */
    evect_push(dest, c);
}


static void write_string(struct evect *dest, const char *s)
{
    /* if the destination is NULL, we're not writing to a variable */
    if (dest == NULL) {
        fputs(s, stdout);
        return;
    }

    /* write to the result buffer when using -v */
    evect_push_string(dest, s);
}


static size_t printf_escape(struct evect *dest, const char *format)
{
    /* handle octal, which has no mandatory prefix */
    if (*format >= '0' && *format <= '7') {
        size_t max_size = 3;

        /* anything above 0o377 doesn't fit in a byte */
        if (*format > '3')
            max_size = 2;

        size_t res;
        ssize_t num_size;
        if ((num_size = parse_integer(&res, 8, format, max_size)) < -1)
            goto not_an_escape;

        assert(res <= 256);
        write_char(dest, res);
        return num_size;
    }

    switch (*format) {
    case '\\':
        write_char(dest, '\\');
        return 1;
    case 'a':
        write_char(dest, '\a');
        return 1;
    case 'b':
        write_char(dest, '\b');
        return 1;
    case 'f':
        write_char(dest, '\f');
        return 1;
    case 'n':
        write_char(dest, '\n');
        return 1;
    case 'r':
        write_char(dest, '\r');
        return 1;
    case 't':
        write_char(dest, '\t');
        return 1;
    case 'v':
        write_char(dest, '\v');
        return 1;
    case 'x': {
        size_t res;
        ssize_t num_size;
        if ((num_size = parse_integer(&res, 16, &format[1], 2)) < 0)
            goto not_an_escape;

        assert(res <= 256);
        write_char(dest, res);
        return 1 + num_size;
    }
    }

not_an_escape:
    write_char(dest, '\\');
    return 0;
}

enum conversion_type
{
    CONV_CHAR = 0, /* c */
    CONV_ESCAPE_STR, /* shell-specific 'b' */
    CONV_STR, /* s */
    CONV_OCTAL, /* o */
    CONV_INT, /* i */
    CONV_UINT, /* u */
    CONV_HEX, /* x */
};

enum sign_policy
{
    /* only print - */
    SIGN_NEG = 0,
    /* pad with a space when there's a + */
    SIGN_SPACE,
    /* always add a sign */
    SIGN_ALWAYS,
};

struct conversion_specifier
{
    /* /!\ conversion specifiers must consume arguments /!\ */
    enum conversion_type type;

    enum sign_policy sign_policy;

    /* use upper chars for hex printing */
    bool upper;

    /* '#' flag: type dependant modifier */
    bool alternative_form;

    /* minimum field width */
    size_t width;

    /* 0 padding for integers, length for strings
    ** SIZE_MAX encodes the default behavior
    */
    size_t precision;

    /* encodes "%.*s" */
    bool dynamic_precision;

    /* either ' ' or '0' */
    char pad_char;

    /* '-' flag */
    bool left_justify;

    /* the number of characters the specifier took to parse, % excluded */
    size_t spec_length;
};


int parse_conversion_specifier(struct conversion_specifier *spec, const char *format)
{
    const char *const format_start = format;

    spec->sign_policy = SIGN_NEG;
    spec->upper = false;
    spec->alternative_form = false;
    spec->width = 0;
    spec->precision = SIZE_MAX;
    spec->dynamic_precision = false;
    spec->pad_char = ' ';
    spec->left_justify = false;

    /* flags */
parse_flag:
    switch (*format) {
    case '-':
        spec->left_justify = true;
    next_flag:
        format++;
        goto parse_flag;
    case '+':
        spec->sign_policy = SIGN_ALWAYS;
        goto next_flag;
    case ' ':
        if (spec->sign_policy != SIGN_ALWAYS)
            spec->sign_policy = SIGN_SPACE;
        goto next_flag;
    case '#':
        spec->alternative_form = true;
        goto next_flag;
    case '0':
        spec->pad_char = '0';
        goto next_flag;
    }

    ssize_t parsed_chars;

    /* field width */
    if ((parsed_chars = parse_integer(&spec->width, 10, format, SIZE_MAX)) < 0)
        return -1;
    format += parsed_chars;

    /* precision */
    if (*format == '.') {
        format++;
        if (*format == '*') {
            spec->dynamic_precision = true;
            format++;
        } else {
            if ((parsed_chars = parse_integer(&spec->precision, 10, format, SIZE_MAX))
                < 0)
                return -1;
            format += parsed_chars;
        }
    }

    /* conversion specifier character */
    switch (*format) {
    case 'c':
        spec->type = CONV_CHAR;
        break;
    case 'b':
        spec->type = CONV_ESCAPE_STR;
        break;
    case 's':
        spec->type = CONV_STR;
        break;
    case 'd':
    case 'i':
        spec->type = CONV_INT;
        break;
    case 'o':
        spec->type = CONV_OCTAL;
        break;
    case 'u':
        spec->type = CONV_UINT;
        break;
    case 'X':
        spec->upper = true;
        /* FALLTHRU */
    case 'x':
        spec->type = CONV_HEX;
        break;
    default:
        /* return an error when something unknown comes up */
        return -1;
    }

    spec->spec_length = format - format_start + /* conversion spec char */ 1;
    return 0;
}

static const char *pop_arg(int *arg_i, int argc, char *argv[])
{
    if (*arg_i == argc)
        return "";

    const char *res = argv[*arg_i];
    (*arg_i)++;
    return res;
}

static void pad(struct evect *dest, const struct conversion_specifier *spec,
                size_t data_width)
{
    if (data_width >= spec->width)
        return;

    for (size_t i = data_width; i < spec->width; i++)
        write_char(dest, spec->pad_char);
}

static void pad_pre(struct evect *dest, const struct conversion_specifier *spec,
                    size_t data_width)
{
    if (spec->left_justify)
        return;
    pad(dest, spec, data_width);
}

static void pad_post(struct evect *dest, const struct conversion_specifier *spec,
                     size_t data_width)
{
    if (!spec->left_justify)
        return;
    pad(dest, spec, data_width);
}

int parse_integer_argument(const char *arg_str, size_t *res, bool *negative)
{
    if (*arg_str == '-') {
        *negative = true;
        arg_str++;
    } else {
        *negative = false;
    }

    /* If the leading character is a single-quote or double-quote,
    ** the value shall be the numeric value in the underlying codeset
    ** of the character following the single-quote or double-quote.
    */
    if (*arg_str == '\'' || *arg_str == '"') {
        *res = arg_str[1];
        return 0;
    }

    size_t basis = 10;

    if (*arg_str == '0') {
        arg_str++;
        basis = 8;
        if (*arg_str == 'x') {
            arg_str++;
            basis = 16;
        }
    }

    return parse_pure_integer(res, basis, arg_str);
}

size_t integer_convertion_props(enum conversion_type type, bool *signed_conv)
{
    *signed_conv = false;
    switch (type) {
    case CONV_OCTAL:
        return 8;
    case CONV_INT:
        *signed_conv = true;
        /* FALLTHRU */
    case CONV_UINT:
        return 10;
    case CONV_HEX:
        return 16;
    default:
        abort();
    }
}

char integer_sign_prefix(enum sign_policy policy, bool negative)
{
    switch (policy) {
    case SIGN_NEG:
        return negative ? '-' : 0;
    case SIGN_SPACE:
        return negative ? '-' : ' ';
    case SIGN_ALWAYS:
        return negative ? '-' : '+';
    default:
        abort();
    }
}


nsh_err_t builtin_printf(struct environment *env, int argc, char *argv[])
{
    if (argc < /* {"printf", "stuff"} */ 2)
        goto print_help;

    /* the result buffer used with -v */
    struct evect result = {0};

    /* the index of the format string argument */
    int format_start = 1;

    /* "printf -v var stuff" stores "stuff" in the variable "var" */
    const char *dest_var = NULL;
    if (strcmp(argv[1], "-v") == 0) {
        if (argc < /* {"printf", "-v", "var", "stuff"} */ 4)
            goto print_help;

        evect_init(&result, 10);
        dest_var = argv[2];
        format_start += 2;
    }

    /* passed as an argument to write_char */
    struct evect *dest = NULL;
    if (dest_var)
        dest = &result;

    const char *format = argv[format_start];
    int arg_i = format_start + 1;

    /* here's the idea: interpret the format string, and restart while there are arguments remaining.
    ** if there aren't enough arguments to complete a cycle, assume empty strings.
    */
    size_t specifiers;
    do {
        specifiers = 0;
        for (int i = 0; format[i]; i++) {
            /* handle \ escapes */
            if (format[i] == '\\') {
                i += printf_escape(dest, &format[i + 1]);
                continue;
            }

            /* handle non-special characters */
            if (format[i] != '%') {
                write_char(dest, format[i]);
                continue;
            }

            /* handle %% escapes.
            ** this isn't processed as a conversion specifier, as it doesn't consume any argument.
            */
            if (format[i + 1] == '%') {
                write_char(dest, '%');
                i++;
                continue;
            }

            /* we met one more specifier, count it in */
            specifiers++;

            /* parse and evaluate the conversion specifier */
            struct conversion_specifier spec;
            if (parse_conversion_specifier(&spec, &format[i + 1])) {
                fprintf(stderr, "%s: invalid conversion specifier\n", argv[0]);
                goto error;
            }

            /* handle %.*s */
            if (spec.dynamic_precision) {
                const char *dynamic_precision_str = pop_arg(&arg_i, argc, argv);
                if (parse_pure_integer(&spec.precision, 10, dynamic_precision_str)
                    == -1) {
                    fprintf(stderr, "%s: argument %d should be a decimal integer\n",
                            argv[0], arg_i - 1);
                    goto error;
                }
            }

            switch (spec.type) {
            case CONV_CHAR: {
                pad_pre(dest, &spec, 1);
                write_char(dest, pop_arg(&arg_i, argc, argv)[0]);
                pad_post(dest, &spec, 1);
                break;
            }
            case CONV_ESCAPE_STR: /* TODO: support this kind of escape */
            case CONV_STR: {
                const char *arg = pop_arg(&arg_i, argc, argv);
                size_t arg_len = strlen(arg);

                /* %.42s or %.*s can limit the printed size */
                if (arg_len > spec.precision)
                    arg_len = spec.precision;

                pad_pre(dest, &spec, arg_len);
                for (size_t i = 0; i < arg_len; i++)
                    write_char(dest, arg[i]);
                pad_post(dest, &spec, arg_len);
                break;
            }
            case CONV_OCTAL:
            case CONV_INT:
            case CONV_UINT:
            case CONV_HEX: {
                /* get the target basis and signedness */
                bool signed_conv;
                size_t target_basis = integer_convertion_props(spec.type, &signed_conv);

                /* fetch the argument and parse it */
                const char *arg_str = pop_arg(&arg_i, argc, argv);
                size_t arg_value;
                bool negative;
                if (parse_integer_argument(arg_str, &arg_value, &negative) == -1) {
                    fprintf(stderr, "%s: argument %d should be an integer\n", argv[0],
                            arg_i - 1);
                    goto error;
                }

                /* disallow signed to unsigned casts */
                if (!signed_conv && negative) {
                    fprintf(stderr, "%s: negative argument with an unsigned conversion\n",
                            argv[0]);
                    goto error;
                }

                /* the X conversion specifier changes the output alphabet a bit */
                const char *target_alphabet = "0123456789abcdef";
                if (spec.upper)
                    target_alphabet = "0123456789ABCDEF";

                /* compute the max number of chars size_t needs for base 8 and greater */
                const size_t size_max_chars = UINT_MAX_CHARS(size_t);

                /* prepare a sufficiently big buffer, and initialize a cursor at the end */
                char int_buf[size_max_chars + /* \0 */ 1];
                char *converted_int = &int_buf[size_max_chars];
                *converted_int = '\0';

                /* convert the number to a string, moving the cursor backwards */
                for (size_t tmp = arg_value; tmp != 0; tmp /= target_basis)
                    *(--converted_int) = target_alphabet[tmp % target_basis];

                /* do some pointer arithmetics to compute the number of written chars */
                const size_t integer_size = &int_buf[size_max_chars] - converted_int;
                assert(integer_size <= size_max_chars);

                /* precision with integers is just 0 padding,
                ** it can't be lower than the number of digits
                */
                size_t precision = spec.precision;
                /* SIZE_MAX encodes the default precision, which is no padding for integers */
                if (precision == SIZE_MAX)
                    precision = 0;

                if (precision < integer_size)
                    precision = integer_size;

                /* force the display of a single 0 */
                if (integer_size == 0 && precision == 0)
                    precision = 1;

                const char *prefix = "";
                if (spec.alternative_form) {
                    /* the alternative form of octal forces a 0 prefix */
                    if (target_basis == 8 && precision <= integer_size)
                        precision = integer_size + 1;
                    if (target_basis == 16)
                        prefix = spec.upper ? "0X" : "0x";
                }

                char sign_prefix = integer_sign_prefix(spec.sign_policy, negative);

                /* compute the output size */
                size_t char_count = 0;
                if (sign_prefix)
                    char_count += /* - */ 1;
                char_count += /* 0x */ strlen(prefix);
                char_count += /* 000042 */ precision;

                pad_pre(dest, &spec, char_count);

                /* write all the components, in order */
                /* the sign, if present */
                if (sign_prefix)
                    write_char(dest, sign_prefix);
                /* the 0x prefix, if present */
                write_string(dest, prefix);
                /* precision 0 padding */
                size_t precision_padding_size = precision - integer_size;
                for (size_t i = 0; i < precision_padding_size; i++)
                    write_char(dest, '0');
                /* the actual data! */
                write_string(dest, converted_int);
                pad_post(dest, &spec, char_count);
                break;
            }
            default:
                abort();
            }

            /* skip the number of parsed characters in the format string itself */
            i += spec.spec_length;
        }

        /* if there are no specifiers, we won't exit the loop unless forcing an exit */
        if (!specifiers)
            break;
    } while (arg_i < argc);

    /* perform the variable assignment if needed */
    if (dest_var)
        environment_var_assign_cstring(env, strdup(dest_var), evect_data(&result), false);

    return NSH_OK;

print_help:
    env->code = printf_help(argv);
    return NSH_OK;

error:
    if (dest)
        evect_destroy(dest);
    env->code = 1;
    return NSH_OK;
}
