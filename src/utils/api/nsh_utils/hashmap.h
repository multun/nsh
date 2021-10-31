#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>


/**
** \brief an intrusive hashmap item
*/
struct hashmap_item
{
    uint32_t hash_key;
    char *key;
    struct hashmap_item **prev;
    struct hashmap_item *next;
};

/**
** \brief A hashmap
*/
struct hashmap
{
    size_t size;
    size_t capacity;
    struct hashmap_item **tab;
};

/**
** \brief computes the hash of a cstring
** \param data the cstring to compute the hash of
** \return the hash of a given string
*/
uint32_t hashmap_hash(const char *data);


/**
** \brief build a new hash table with initial capacity.
** \param htab the table to initialize
** \param capacity the initial size of the htable
** \return the allocated htable
*/
void hashmap_init(struct hashmap *htab, size_t capacity);

static inline void hashmap_item_init(struct hashmap_item *head, char *key)
{
    head->key = key;
    head->hash_key = hashmap_hash(key);
    head->prev = NULL;
    head->next = NULL;
}

/**
** \brief find the node corresponding to a given key
** \param htab the hash table to look into
** \param insertion_point the place to insert if find fails
** \param key the key to look for
** \return pointer to node
*/
struct hashmap_item *hashmap_find(struct hashmap *htab,
                               struct hashmap_item ***insertion_point,
                               const char *key);

/**
** \brief add an entry at the given location
** \param insertion_point where to insert
** \param head the element to insert
*/
void hashmap_insert(struct hashmap *htab, struct hashmap_item **insertion_point, struct hashmap_item *head);

/**
** \brief remove the given item from the hash table
** \param head the element to remove
*/
void hashmap_remove(struct hashmap *htab, struct hashmap_item *head);

/**
** \brief delete the table, but don't free hash_heads ! (it's an intrusive collection)
** \param hashmap the hash table to destroy
*/
void hashmap_destroy(struct hashmap *hashmap);

/**
** \brief map a function on every node of the hashmap
** \param hashmap the hashmap to apply func on
** \param func the function to apply on each node
*/
void hashmap_apply(struct hashmap *hashmap, void (*mapper)(struct hashmap_item *ptr));
