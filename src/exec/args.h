#pragma once

/**
** \brief free an argv array
*/
static inline void argv_free(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
}
