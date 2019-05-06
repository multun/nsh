#pragma once

/**
** \brief counts the number of arguments inside an argv array
*/
int argv_count(char **argv);

/**
** \brief clones an argv array
** \details also clones all the strings inside the array
*/
char **argv_dup(char **argv);

/**
** \brief free an argv array
*/
void argv_free(char **argv);
