#include <nsh_exec/shopt.h>
#include <nsh_exec/environment.h>
#include <nsh_utils/evect.h>

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


#define STRING_LIST(Enum, StrRepr) StrRepr,


static const char *g_shopt_tab[SHOPT_COUNT] = {SHOPTS_APPLY(STRING_LIST)};


enum shopt shopt_from_string(const char *str)
{
    for (size_t i = 0; i < SHOPT_COUNT; i++)
        if (!strcmp(str, g_shopt_tab[i]))
            return i;
    return SHOPT_COUNT;
}

const char *string_from_shopt(size_t index)
{
    if (index >= SHOPT_COUNT)
        return NULL;
    return g_shopt_tab[index];
}

enum shopt_action
{
    SHOPT_ACTION_PRINT = 0,
    SHOPT_ACTION_SET = 1,
    SHOPT_ACTION_UNSET = 2,
};


struct shopt_options
{
    bool quiet;
    enum shopt_action action;
    int pos_args_start;
};


static bool parse_builtin_shopt_opt(struct shopt_options *res, int argc, char **argv)
{
    memset(res, 0, sizeof(*res));

    int i = 1;
    for (; i < argc; i++) {
        char *cur_arg = argv[i];
        if (cur_arg[0] != '-')
            break;

        if (strlen(cur_arg) != 2) {
            warnx("shopt: invalid argument: %s", cur_arg);
            return false;
        }

        switch (cur_arg[1]) {
        case 'q':
            res->quiet = true;
            break;
        case 's':
            res->action = SHOPT_ACTION_SET;
            break;
        case 'u':
            res->action = SHOPT_ACTION_UNSET;
            break;
        default:
            warnx("shopt: invalid argument: %s", cur_arg);
            return false;
        }
    }

    res->pos_args_start = i;
    return true;
}

static void print_shopt(struct environment *env, int index)
{
    const char *name = string_from_shopt(index);
    bool value = env->shopts[index];
    printf("%s\t%s\n", name, value ? "on" : "off");
}

static void print_shopts(struct environment *env, struct shopt_options *options, int argc,
                         char **argv)
{
    if (options->quiet)
        return;

    if (options->pos_args_start != argc) {
        for (int i = options->pos_args_start; i < argc; i++)
            print_shopt(env, shopt_from_string(argv[i]));
    } else {
        // by default, print the status of all options
        for (size_t i = 0; i < SHOPT_COUNT; i++)
            print_shopt(env, i);
    }
}

int builtin_shopt(struct environment *env, struct exception_catcher *catcher __unused,
                  int argc, char **argv)
{
    struct shopt_options opt;
    if (!parse_builtin_shopt_opt(&opt, argc, argv))
        return 2;

    for (int i = opt.pos_args_start; i < argc; i++)
        if (shopt_from_string(argv[i]) == SHOPT_COUNT) {
            warnx("shopt: %s: invalid shell option name", argv[i]);
            return 1;
        }

    if (opt.action == SHOPT_ACTION_PRINT) {
        print_shopts(env, &opt, argc, argv);
        return 0;
    }

    bool value = opt.action == SHOPT_ACTION_SET;
    for (int i = opt.pos_args_start; i < argc; i++)
        env->shopts[shopt_from_string(argv[i])] = value;
    return 0;
}
