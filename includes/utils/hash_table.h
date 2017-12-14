#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct pair
{
  uint32_t hkey;
  char *key;
  void *value;
  struct pair *next;
};

typedef struct htable
{
  size_t size;
  size_t capacity;
  struct pair **tab;
} s_htable;


/*
 * htable_create(capacity):
 * build a new hash table with initial capacity.
 */
s_htable *htable_create(size_t capacity);

/*
 * htable_access(htable, key):
 * returns a pointer to the value containing the given key
 */
struct pair *htable_access(s_htable *htable, const char *key);

/*
 * htable_add(htable,key,value):
 * add the pair (key,value) to the hash table
 */
bool htable_add(s_htable *htable, char *key, void *value);

/*
 * htable_remove(htable, key):
 * removes the pair containing the given key from the hash table
 */
void htable_remove(s_htable *htable, char *key);

/*
 * htable_clear(htable):
 * delete all pairs in the table
 */
void htable_free(s_htable *htable);


void htable_map(s_htable *htable, void (*func)(struct pair *ptr));
