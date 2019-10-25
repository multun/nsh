#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <err.h>

#include "shexec/builtins.h"
#include "utils/alloc.h"
#include "utils/hash_table.h"
#include "shlex/variable.h"
#include "shexec/builtin_cd.h"

void update_pwd(bool oldpwd, struct environment *env)
{
    const size_t size = PATH_MAX;
    char *buf = xcalloc(size, sizeof(char));
    if (!getcwd(buf, size)) {
        free(buf);
        buf = NULL;
        return;
    }
    char *pwd = strdup(oldpwd ? "OLDPWD" : "PWD");
    environment_var_assign(env, pwd, buf, true);
}

static int cd_from_env(const char *env_var, struct environment *env, bool save)
{
    struct pair *p = htable_access(env->vars, env_var);
    char *path = NULL;
    if (p && p->value) {
        struct variable *var = p->value;
        path = var->value;
    }

    if (!path) {
        warnx("cd: no %s set", env_var);
        return 1;
    }

    if (save)
        path = strdup(path);

    update_pwd(true, env);
    if (chdir(path) != 0) {
        warn("cd: chdir failed");
        return 1;
    }
    if (save)
        free(path);

    update_pwd(false, env);
    return 0;
}

static int cd_with_minus(struct environment *env)
{
    int res = cd_from_env("OLDPWD", env, true);

    if (!res) {
        char *buf = xcalloc(PATH_MAX, sizeof(char));
        size_t size = PATH_MAX;
        if (!getcwd(buf, size)) {
            free(buf);
            return 1;
        }
        printf("%s\n", buf);
        free(buf);
    }
    return res;
}

int builtin_cd(struct environment *env, struct errcont *cont, int argc, char **argv)
{
    if (!env || !cont)
        warnx("cd: missing context elements");

    if (argc > 2) {
        warnx("cd: too many arguments");
        return 1;
    }

    if (argc == 1)
        return cd_from_env("HOME", env, false);
    else if (!strcmp(argv[1], "-"))
        return cd_with_minus(env);
    else {
        update_pwd(true, env);
        if (chdir(argv[1]) != 0) {
            warn("cd: chdir failed");
            return 1;
        }
        update_pwd(false, env);
    }
    return 0;
}
