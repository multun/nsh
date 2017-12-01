#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utils/alloc.h"
#include "utils/hash_table.h"


#define RESIZE_GROWTH   2
#define RESIZE_TRIGGER  0.75


static uint32_t hash(const char *data)
{
  const void *vdata = data;
  const uint8_t *cur = vdata;
  uint32_t h = 0;
  for (size_t n = 0; cur[n]; n++)
  {
    h += cur[n];
    h += h * 1024;
    h = h ^ (h / 64);
  }
  h += h * 8;
  h = h ^ (h / 2048);
  return h + h * 32768;
}


static struct pair **pair_insertpos(struct htable *table, char *key)
{
  return &table->tab[hash(key) % table->capacity];
}


static void expand_table(struct htable *htable)
{
  size_t former_cap = htable->capacity;
  htable->capacity *= RESIZE_GROWTH;

  struct pair **ftab = htable->tab;
  htable->tab = xcalloc(htable->capacity, sizeof(struct pair*));

  for (size_t i = 0; i < former_cap; i++)
    for (struct pair *fp = ftab[i]; fp; fp = fp->next)
    {
      struct pair **ipos = pair_insertpos(htable, fp->key);
      fp->next = *ipos;
      *ipos = fp;
    }
  free(ftab);
}


struct htable *htable_create(size_t capacity)
{
  struct htable *ret = xmalloc(sizeof(struct htable));
  ret->size = 0;
  ret->capacity = capacity;
  ret->tab = xcalloc(capacity, sizeof(struct pair*));
  return ret;
}


void *htable_access(struct htable *htable, char *key)
{
  struct pair *ret = *pair_insertpos(htable, key);

  for (; ret; ret = ret->next)
    if (!strcmp(key, ret->key))
      return ret->value;

  return NULL;
}


static struct pair *alloc_pair(char *key, void *value)
{
  struct pair *ret = xmalloc(sizeof(struct pair));
  ret->key = key;
  ret->value = value;
  ret->next = NULL;
  return ret;
}


bool htable_add(struct htable *htable, char *key, void *value)
{
  if (htable->size + 1 > htable->capacity * RESIZE_TRIGGER)
    expand_table(htable);

  struct pair **ipos = pair_insertpos(htable, key);
  for (struct pair *p = *ipos; p; p = p->next)
    if (!strcmp(p->key, key))
      return false;

  htable->size++;

  struct pair *nelem = alloc_pair(key, value);
  nelem->next = *ipos;
  *ipos = nelem;
  return true;
}


void htable_remove(struct htable *htable, char *key)
{
  struct pair **ipos = pair_insertpos(htable, key);
  for (; *ipos; ipos = &(*ipos)->next)
    if (!strcmp((*ipos)->key, key))
    {
      struct pair *rpair = *ipos;
      *ipos = (*ipos)->next;
      free(rpair);
      htable->size--;
      break;
    }
}


void htable_clear(struct htable *htable)
{
  for (size_t i = 0; i < htable->capacity; i++)
  {
    struct pair *pp = NULL, *fp = htable->tab[i];
    while (fp)
    {
      free(pp);
      pp = fp;
      fp = fp->next;
    }
    free(pp);
  }

  memset(htable->tab, 0, sizeof(struct pair*) * htable->capacity);
  htable->size = 0;
}
