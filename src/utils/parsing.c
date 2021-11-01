#include <nsh_utils/parsing.h>
#include <stdint.h>

// the field is zero if the char doesn't map to anything, 1 + value otherwise
static char arith_number_map[] = {
    ['0'] = 1,  ['1'] = 2,  ['2'] = 3,  ['3'] = 4,  ['4'] = 5,  ['5'] = 6,
    ['6'] = 7,  ['7'] = 8,  ['8'] = 9,  ['9'] = 10, ['A'] = 11, ['B'] = 12,
    ['C'] = 13, ['D'] = 14, ['E'] = 15, ['F'] = 16, ['a'] = 11, ['b'] = 12,
    ['c'] = 13, ['d'] = 14, ['e'] = 15, ['f'] = 16,
};

int parse_digit(int c)
{
    int max_ascii = sizeof(arith_number_map) / sizeof(arith_number_map[0]);
    if (c <= 0 || c >= max_ascii)
        return -1;

    return arith_number_map[c] - 1;
}


static int parse_integer_rec(size_t *res, size_t *res_size, size_t basis, const char *str,
                             size_t max_length)
{
    /* recursion end cases */

    /* stop if we reached max length */
    if (max_length == 0)
        return 0;

    int digit = parse_digit(*str);
    /* stop if this char isn't a digit */
    if (digit == -1)
        return 0;

    /* stop if this char is not part of the basis */
    if ((size_t)digit >= basis)
        return 0;

    /* *res *= basis */
    if (__builtin_mul_overflow(*res, basis, res))
        return -1;

    /* *res += digit */
    if (__builtin_add_overflow(*res, digit, res))
        return -1;

    /* the result took one more char to parse */
    (*res_size)++;

    /* return whether an error occured */
    return parse_integer_rec(res, res_size, basis, str + 1, max_length - 1);
}


int parse_integer(size_t *res, size_t basis, const char *str, size_t max_length)
{
    *res = 0;
    size_t res_size = 0;
    if (parse_integer_rec(res, &res_size, basis, str, max_length) == -1)
        return -1;

    return res_size;
}


int parse_pure_integer(size_t *res, size_t basis, const char *str)
{
    int res_size = parse_integer(res, basis, str, SIZE_MAX);
    if (res_size == -1)
        return -1;

    if (str[res_size] != '\0')
        return -1;

    return res_size;
}
