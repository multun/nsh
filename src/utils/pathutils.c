#include "utils/pathutils.h"

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *home_suffix(const char *suffix)
{
    char *home = getpwuid(getuid())->pw_dir;
    char *path = malloc(strlen(home) + strlen(suffix) + 1);
    return strcat(strcpy(path, home), suffix);
}
