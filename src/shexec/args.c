#include "utils/alloc.h"
#include "shexec/args.h"

#include <stdlib.h>
#include <string.h>

void argv_free(char **argv)
{
    for (size_t i = 0; argv[i]; i++)
        free(argv[i]);
    free(argv);
}
