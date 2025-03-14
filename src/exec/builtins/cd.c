#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <nsh_exec/environment.h>
#include <nsh_utils/alloc.h>
#include <nsh_utils/mprintf.h>
#include <nsh_utils/hashmap.h>
#include <nsh_lex/variable.h>
#include <nsh_utils/strutils.h>
#include <nsh_utils/pathutils.h>
#include <nsh_utils/cpvect.h>
#include <nsh_utils/safe_syscalls.h>


struct cd_options
{
    int args_start;
    bool physical;
    bool previous_dir;
};


static int parse_arguments(struct cd_options *opts, int argc, char *argv[])
{
    /* parse arguments */
    opts->args_start = 1;
    opts->physical = false;
    opts->previous_dir = false;
    for (; opts->args_start < argc; opts->args_start++) {
        const char *cur_arg = argv[opts->args_start];
        if (cur_arg[0] != '-')
            break;

        /* stop on cd - */
        if (cur_arg[1] == '\0') {
            opts->previous_dir = true;
            break;
        }

        for (int i = 1; cur_arg[i]; i++) {
            char option = cur_arg[i];

            if (option == '-')
                break;
            if (option == 'P')
                opts->physical = true;
            else if (option == 'L')
                opts->physical = false;
            else {
                warnx("%s: unknown option: %c", argv[0], option);
                return 1;
            }
        }
    }
    return 0;
}


nsh_err_t builtin_cd(struct environment *env, int argc, char *argv[])
{
    /* The standard cd algorithm is way more complicated than you'd think.
       It's probably implemented wrong, but it least it isn't too unreadable.
       https://pubs.opengroup.org/onlinepubs/9699919799/utilities/cd.html */

    int rc;
    char *curpath = NULL;

    struct cd_options options;
    if ((rc = parse_arguments(&options, argc, argv)))
        goto error;

    if ((argc - options.args_start) > 1) {
        warnx("%s: too many arguments", argv[0]);
        goto error;
    }

    const char *directory = argv[options.args_start];

    if (options.previous_dir) {
        curpath = strdup(environment_var_get_cstring(env, "OLDPWD"));
        goto process_curpath;
    }

    /* handle the cd to home case */
    if (directory == NULL) {
        const char *HOME = environment_var_get_cstring(env, "HOME");
        /* step 1 */
        if (HOME == NULL) {
            warnx("%s: HOME not set", argv[0]);
            goto error;
        }
        /* step 2 */
        directory = HOME;
    }

    /* step 3 */
    if (directory[0] == '/') {
        curpath = strdup(directory);
        goto process_curpath;
    }

    /* step 4 */
    if (startswith(directory, "./") || startswith(directory, "../")) {
        curpath = strdup(directory);
        goto process_curpath;
    }

    /* CDPATH is just like PATH, but for cd! */
    const char *CDPATH = environment_var_get_cstring(env, "CDPATH");
    if (CDPATH) {
        /* for each directory in CDPATH */
        struct pathlist_iter it;
        for (pathlist_iter_init(&it, CDPATH); pathlist_iter_valid(&it);
             pathlist_iter_next(&it)) {
            if (it.length == 0)
                /* posix says an empty element means the current directory should be searched.
                   this is easy to implement, but too risky for the end user for us to implement. */
                continue;

            const char *spacer = "/";
            if (it.data[it.length - 1] == '/')
                spacer = "";

            /* when doing cd DIR, if candidate_cdpath_dir/DIR exists, cd into it */
            char *candidate =
                mprintf("%.*s%s%s", (int)it.length, it.data, spacer, directory);
            struct stat64 statbuf;
            if (stat64(candidate, &statbuf) >= 0 && S_ISDIR(statbuf.st_mode)) {
                curpath = candidate;
                goto process_curpath;
            }
            free(candidate);
        }
    }

    /* step 6 */
    curpath = strdup(directory);

    const char *PWD;

process_curpath:
    PWD = environment_var_get_cstring(env, "PWD");

    /* step 7 */
    if (!options.physical) {
        /* if the current target isn't an absolute path, prepend PWD */
        if (PWD && curpath[0] != '/') {
            char *abs_path = path_join(PWD, curpath);
            if (abs_path) {
                free(curpath);
                curpath = abs_path;
            }
        }

        /* step 8 */
        char *canonical_curpath = path_canonicalize(curpath);
        free(curpath);
        curpath = canonical_curpath;

        /* TODO: step 9*/
    }

    /* step10 */
    if (chdir(curpath)) {
        warnx("%s: chdir(%s) failed", argv[0], curpath);
        goto error;
    }

    /* move PWD to OLDPWD */
    if (PWD)
        environment_var_assign(env, strdup("OLDPWD"),
                               &sh_string_create(strdup(PWD))->base, true);

    /* compute the path to store back */
    char *newpwd;
    if (options.physical) {
        if ((newpwd = safe_getcwd()) == NULL)
            warn("%s: getcwd() failed", argv[0]);
    } else {
        newpwd = curpath;
        curpath = NULL;
    }

    /* store the new PWD into the environment */
    if (newpwd)
        environment_var_assign(env, strdup("PWD"), &sh_string_create(newpwd)->base, true);

    /* Success */
    rc = 0;

cleanup:
    free(curpath);
    env->code = rc;
    return NSH_OK;

error:
    rc = 1;
    goto cleanup;
}
