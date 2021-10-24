#include <nsh_utils/alloc.h>
#include <nsh_exec/args.h>

#include <stdlib.h>
#include <string.h>

void argv_free(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}
