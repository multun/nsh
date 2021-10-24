#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <nsh_utils/alloc.h>
#include <nsh_utils/hash_table.h>

#define RESIZE_GROWTH 2
#define RESIZE_TRIGGER 0.75

uint32_t hash_table_hash(const char *data)
{
    const void *vdata = data;
    const uint8_t *cur = vdata;
    uint32_t h = 0;
    for (size_t n = 0; cur[n]; n++) {
        h += cur[n];
        h += h * 1024;
        h = h ^ (h / 64);
    }
    h += h * 8;
    h = h ^ (h / 2048);
    return h + h * 32768;
}

static struct hash_head **head_tab_insertpos(struct hash_table *table, const char *key)
{
    return &table->tab[hash_table_hash(key) % table->capacity];
}

static void hash_table_insert_noincr(struct hash_head **insertion_point, struct hash_head *head)
{
    // initialize the new node
    head->next = *insertion_point;
    head->prev = insertion_point;

    // link the previous node to the new node
    *head->prev = head;
    // link the next node, if any, to the new node
    if (head->next)
        head->next->prev = &head->next;
}

static void expand_table(struct hash_table *htable)
{
    size_t former_cap = htable->capacity;
    struct hash_head **former_tab = htable->tab;

    htable->capacity *= RESIZE_GROWTH;
    htable->tab = xcalloc(htable->capacity, sizeof(struct hash_head *));

    for (size_t i = 0; i < former_cap; i++)
        for (struct hash_head *cur = former_tab[i]; cur;) {
            struct hash_head **ipos = head_tab_insertpos(htable, cur->key);
            struct hash_head *tmp = cur->next;
            hash_table_insert_noincr(ipos, cur);
            cur = tmp;
        }
    free(former_tab);
    hash_table_check(htable);
}

void hash_table_init(struct hash_table *htab, size_t capacity)
{
    htab->size = 0;
    htab->capacity = capacity;
    htab->tab = xcalloc(capacity, sizeof(struct hash_head *));
}

struct hash_head *hash_table_find(struct hash_table *htab, struct hash_head ***insertion_point, const char *key)
{
    if (htab->size + 1 > htab->capacity * RESIZE_TRIGGER)
        expand_table(htab);

    struct hash_head **ipos = head_tab_insertpos(htab, key);

    if (insertion_point)
        *insertion_point = ipos;

    for (struct hash_head *cur = *ipos; cur; cur = cur->next)
        if (strcmp(cur->key, key) == 0)
            return cur;

    hash_table_check(htab);
    return NULL;
}

void hash_table_insert(struct hash_table *htab, struct hash_head **insertion_point, struct hash_head *head)
{
    htab->size++;
    hash_table_check(htab);
    hash_table_insert_noincr(insertion_point, head);
    hash_table_check(htab);
}

void hash_table_remove(struct hash_table *htab, struct hash_head *head)
{
    assert(htab->size > 0);
    htab->size--;

    // link the previous node the the next one
    *head->prev = head->next;
    // link the next node, if any, to the previous one
    if (head->next)
        head->next->prev = head->prev;

    head->prev = NULL;
    head->next = NULL;
    hash_table_check(htab);
}

void hash_table_destroy(struct hash_table *htab)
{
    free(htab->tab);
}

void hash_table_map(struct hash_table *htab, void (*mapper)(struct hash_head *ptr))
{
    for (size_t i = 0; i < htab->capacity; i++) {
        struct hash_head *next;
        for (struct hash_head *cur = htab->tab[i]; cur; cur = next)
        {
            next = cur->next;
            if (cur)
                mapper(cur);
        }
    }
}
