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

struct htable
{
  size_t size;
  size_t capacity;
  struct pair **tab;
};


/*
 * htable_create(capacity):
 * build a new hash table with initial capacity.
 */
struct htable *htable_create(size_t capacity);

/*
 * htable_access(htable, key):
 * returns a pointer to the value containing the given key
 */
void *htable_access(struct htable *htable, char *key);

/*
 * htable_add(htable,key,value):
 * add the pair (key,value) to the hash table
 */
bool htable_add(struct htable *htable, char *key, void *value);

/*
 * htable_remove(htable, key):
 * removes the pair containing the given key from the hash table
 */
void htable_remove(struct htable *htable, char *key);

/*
 * htable_clear(htable):
 * delete all pairs in the table
 */
void htable_clear(struct htable *htable);
