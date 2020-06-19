#include "utils/parsing.h"


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

int parse_digit(int c)
{
    int max_ascii = sizeof(arith_number_map) / sizeof(arith_number_map[0]);
    if (c <= 0 || c >= max_ascii)
        return -1;

    return arith_number_map[c] - 1;
}
