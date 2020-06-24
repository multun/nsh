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

void update_pwd(const char *var, struct environment *env)
{
    const size_t size = PATH_MAX;
    char *buf = xcalloc(size, sizeof(char));
    if (!getcwd(buf, size)) {
        free(buf);
        buf = NULL;
        return;
    }
    environment_var_assign(env, strdup(var), buf, true);
}

static int cd(struct environment *env, const char *path)
{
    update_pwd("OLDPWD", env);
    if (chdir(path) != 0) {
        warn("cd: chdir failed");
        return 1;
    }
    update_pwd("PWD", env);
    return 0;
}

static int cd_from_env(const char *env_var, struct environment *env)
{
    const char *env_val = environment_var_get(env, env_var);
    if (env_val == NULL) {
        warnx("cd: '%s' not set", env_var);
        return 1;
    }

    cd(env, env_val);
    return 0;
}

static int cd_with_minus(struct environment *env)
{
    int res = cd_from_env("OLDPWD", env);

    if (res)
        return res;

    char *buf = xcalloc(PATH_MAX, sizeof(char));
    size_t size = PATH_MAX;
    if (!getcwd(buf, size)) {
        free(buf);
        return 1;
    }
    printf("%s\n", buf);
    free(buf);
    return 0;
}

int builtin_cd(struct environment *env, struct ex_scope *ex_scope __unused, int argc, char **argv)
{
    if (argc > 2) {
        warnx("cd: too many arguments");
        return 1;
    }

    if (argc == 1)
        return cd_from_env("HOME", env);
    else if (!strcmp(argv[1], "-"))
        return cd_with_minus(env);
    else
        return cd(env, argv[1]);
    return 0;
}
