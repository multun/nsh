#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
** \brief an intrusive hash table
*/
struct hash_head
{
    uint32_t hash_key;
    char *key;
    struct hash_head **prev;
    struct hash_head *next;
};

static inline struct hash_head **hash_head_insertion_point(struct hash_head *head)
{
    return head->prev;
}

static inline char *hash_head_key(struct hash_head *head)
{
    return head->key;
}

uint32_t hash_table_hash(const char *data);

static inline void hash_head_init(struct hash_head *head, char *key)
{
    head->key = key;
    head->hash_key = hash_table_hash(key);
    head->prev = NULL;
    head->next = NULL;
}

/**
** \brief htable structure
*/
struct hash_table
{
    size_t size;
    size_t capacity;
    struct hash_head **tab;
};

#include <assert.h>

static inline void hash_table_check(struct hash_table *table)
{
    for (size_t i = 0; i < table->size; i++)
    {
        struct hash_head **prev = &table->tab[i];
        for (struct hash_head *cur = *prev; cur; prev = &cur->next, cur = cur->next)
        {
            assert(cur->prev = prev);
            assert(cur->hash_key == hash_table_hash(cur->key));
        }
    }
}

struct hash_table_it
{
    size_t i;
    struct hash_head *cur;
};

#define HASH_TABLE_IT_INIT (struct hash_table_it) { .i = 0 }

#define for_each_hash(It, Hash)                  \
    for ((It) = HASH_TABLE_IT_INIT;              \
         (It).i < (Hash)->capacity;              \
         (It).i++)                               \
        for ((It).cur = (Hash)->tab[(It).i];     \
             (It).cur;                           \
             (It).cur = (It).cur->next)

struct hash_table_safe_it
{
    size_t i;
    struct hash_head *cur;
    struct hash_head *tmp;
};

#define HASH_TABLE_SAFE_IT_INIT  (struct hash_table_safe_it) { .i = 0 }

#define for_each_hash_safe(It, Hash)                    \
    for ((It) = HASH_TABLE_SAFE_IT_INIT;                \
         (It).i < (Hash)->capacity;                     \
         (It).i++)                                      \
        for ((It).cur = (Hash)->tab[(It).i];            \
             (It).cur;                                  \
             (It).cur = (It).tmp)                       \
            if (((It).tmp = (It).cur->next) || 1)


/**
** \brief build a new hash table with initial capacity.
** \param htab the table to initialize
** \param capacity the initial size of the htable
** \return the allocated htable
*/
void hash_table_init(struct hash_table *htab, size_t capacity);

/**
** \brief find the node corresponding to a given key
** \param htab the hash table to look into
** \param insertion_point the place to insert if find fails
** \param key the key to look for
** \return pointer to node
*/
struct hash_head *hash_table_find(struct hash_table *htab,
                                  struct hash_head ***insertion_point,
                                  const char *key);

/**
** \brief add an entry at the given location
** \param insertion_point where to insert
** \param head the element to insert
*/
void hash_table_insert(struct hash_table *htab, struct hash_head **insertion_point, struct hash_head *head);

/**
** \brief remove the given item from the hash table
** \param head the element to remove
*/
void hash_table_remove(struct hash_table *htab, struct hash_head *head);

/**
** \brief delete the table, but don't free hash_heads ! (it's an intrusive collection)
** \param hash_table the hash table to destroy
*/
void hash_table_destroy(struct hash_table *hash_table);

/**
** \brief map a function on every node of the hash_table
** \param hash_table the hash_table to apply func on
** \param func the function to apply on each node
*/
void hash_table_map(struct hash_table *hash_table, void (*mapper)(struct hash_head *ptr));
