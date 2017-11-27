#pragma-once

#include <stdbool.h>


typedef struct wordlist
{
  char *str;
  size_t size;
  bool split;
  bool expand;
  struct wordlist *next;
} s_wordlist;


s_wordlist *wordlist_create(char *str, size_t size, bool split, bool expand);
