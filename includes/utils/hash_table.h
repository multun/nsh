#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
** \brief node of the htable
*/
struct pair
{
    uint32_t hkey;
    char *key;
    void *value;
    struct pair *next;
};

/**
** \brief htable structure
*/
typedef struct htable
{
    size_t size;
    size_t capacity;
    struct pair **tab;
} s_htable;

/**
** \brief build a new hash table with initial capacity.
** \param capacity the initial size of the htable
** \return the allocated htable
*/
s_htable *htable_create(size_t capacity);

/**
** \brief find the node corresponding t oa given key
** \param htable the hatable to look into
** \param key the key to match
** \return pointer to node
*/
struct pair *htable_access(s_htable *htable, const char *key);

/**
** \brief add the pair (key,value) to the hash table
** \param htable the hatable to insert into
** \param key the key of the node
** \param value the value of the node
** \return true on success
*/
bool htable_add(s_htable *htable, char *key, void *value);

/**
** \brief removes the pair containing the given key from the hash table
** \param htable the hatable to remove from
** \param key the key of the node to remove.
*/
void htable_remove(s_htable *htable, char *key);

/**
** \brief delete all pairs in the table
** \param htable the htable to remove from
*/
void htable_free(s_htable *htable);

/**
** \brief map a function on every node of the htable
** \param htable the htable to apply func on
** \param func the function to apply on each node
*/
void htable_map(s_htable *htable, void (*func)(struct pair *ptr));
