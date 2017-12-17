#pragma once

/**
** \brief finds the next delimiter in an array of strings
** \param str the array of strings to look into
** \param end the end of the array
** \param delim a collection of delimiters to look for
** \param first choose whether the function returns the first delimiter or not
** \return return either the start of the token or the delimiter
*/
char **strsplit_r(char **str, char **end, const char **delim, bool first);
