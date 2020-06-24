#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cli/shopt.h"
#include "shexec/builtins.h"
#include "utils/evect.h"
#include "utils/parsing.h"

enum shecho_opt
{
    SHECHO_NL = 1,
    SHECHO_ESC = 2,
};

static int parse_options(int *opt, char **argv)
{
    int pos = 1;
    while (argv[pos]) {
        if (argv[pos][0] != '-')
            break;

        if (strlen(argv[pos]) != 2)
            break;

        switch (argv[pos][1]) {
        case 'n':
            *opt |= SHECHO_NL;
            break;
        case 'e':
            *opt |= SHECHO_ESC;
            break;
        case 'E':
            *opt &= ~SHECHO_ESC;
            break;
        default:
            return pos;
        }
        pos++;
    }
    return pos;
}

static size_t echo_parse_base(const char * const str, int base, size_t len)
{
    /* the 'x' or '0' in '\x' or '\0'*/
    char basis_tag = *str;

    char res = 0;

    size_t index = 0;
    for (; index < len; index++) {
        /* the +1 skips the basis tag */
        int num = parse_digit(str[index + 1]);
        /* if the digit isn't a valid input, stop there */
        if (num == -1)
            break;

        res *= base;
        res += num;
    }

    /* handle empty escapes */
    if (index == 0) {
        if (base == 8) {
            /* echo -e '\0' is interpreted as a nul byte, not as an empty escape */
            putchar('\0');
            return 0;
        }
        putchar('\\');
        putchar(basis_tag);
        return 0;
    }

    putchar(res);
    /* we return the number of consumed chars, excluding the basis tag */
    return index;
}

static void echo_print_esc(const char **c)
{
    switch (**c) {
    case 'a':
        putchar('\a');
        break;
    case 'b':
        putchar('\b');
        break;
    case 'c':
        putchar('\0');
        break;
    case 'e':
    case 'E':
        putchar(0x1b);
        break;
    case 'f':
        putchar('\f');
        break;
    case 'n':
        putchar('\n');
        break;
    case 'r':
        putchar('\r');
        break;
    case 't':
        putchar('\t');
        break;
    case 'v':
        putchar('\v');
        break;
    case '\\':
        putchar('\\');
        break;
    case '0':
        *c += echo_parse_base(*c, 8, 3);
        break;
    case 'x':
        *c += echo_parse_base(*c, 16, 2);
        break;
    default:
        putchar('\\');
        putchar(**c);
        break;
    }
}

static void echo_print(const char *c, int opt)
{
    for (; *c; c++) {
        if (!(opt & SHECHO_ESC)) {
            putchar(*c);
            continue;
        }

        if (*c != '\\') {
            putchar(*c);
            continue;
        }

        c++;
        if (*c == '\0') {
            putchar('\\');
            return;
        }

        echo_print_esc(&c);
    }
}

int builtin_echo(struct environment *env __unused, struct ex_scope *ex_scope __unused, int argc, char **argv)
{
    int options = 0;
    if (g_shopts[SHOPT_XPG_ECHO])
        options = SHECHO_ESC;

    int options_end = parse_options(&options, argv);

    for (int i = options_end; i < argc; i++) {
        if (i != options_end)
            putchar(' ');
        echo_print(argv[i], options);
    }

    if (!(options & SHECHO_NL))
        putchar('\n');

    return 0;
}
