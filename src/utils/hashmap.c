#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <nsh_utils/alloc.h>
#include <nsh_utils/hashmap.h>


#define RESIZE_GROWTH 2
#define RESIZE_TRIGGER 0.75


static void hashmap_check(struct hashmap *table)
{
    for (size_t i = 0; i < table->size; i++) {
        struct hashmap_item **prev = &table->tab[i];
        for (struct hashmap_item *cur = *prev; cur; prev = &cur->next, cur = cur->next) {
            assert(cur->prev == prev);
            assert(cur->hash_key == hashmap_hash(cur->key));
        }
    }
}

uint32_t hashmap_hash(const char *data)
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

static struct hashmap_item **head_tab_insertpos(struct hashmap *table, const char *key)
{
    return &table->tab[hashmap_hash(key) % table->capacity];
}

static void hashmap_insert_noincr(struct hashmap_item **insertion_point, struct hashmap_item *head)
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

static void expand_table(struct hashmap *htable)
{
    size_t former_cap = htable->capacity;
    struct hashmap_item **former_tab = htable->tab;

    htable->capacity *= RESIZE_GROWTH;
    htable->tab = xcalloc(htable->capacity, sizeof(struct hashmap_item *));

    for (size_t i = 0; i < former_cap; i++)
        for (struct hashmap_item *cur = former_tab[i]; cur;) {
            struct hashmap_item **ipos = head_tab_insertpos(htable, cur->key);
            struct hashmap_item *tmp = cur->next;
            hashmap_insert_noincr(ipos, cur);
            cur = tmp;
        }
    free(former_tab);
    hashmap_check(htable);
}

void hashmap_init(struct hashmap *htab, size_t capacity)
{
    htab->size = 0;
    htab->capacity = capacity;
    htab->tab = xcalloc(capacity, sizeof(struct hashmap_item *));
}

struct hashmap_item *hashmap_find(struct hashmap *htab, struct hashmap_item ***insertion_point, const char *key)
{
    if (htab->size + 1 > htab->capacity * RESIZE_TRIGGER)
        expand_table(htab);

    struct hashmap_item **ipos = head_tab_insertpos(htab, key);

    if (insertion_point)
        *insertion_point = ipos;

    for (struct hashmap_item *cur = *ipos; cur; cur = cur->next)
        if (strcmp(cur->key, key) == 0)
            return cur;

    hashmap_check(htab);
    return NULL;
}

void hashmap_insert(struct hashmap *htab, struct hashmap_item **insertion_point, struct hashmap_item *head)
{
    htab->size++;
    hashmap_check(htab);
    hashmap_insert_noincr(insertion_point, head);
    hashmap_check(htab);
}

void hashmap_remove(struct hashmap *htab, struct hashmap_item *head)
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
    hashmap_check(htab);
}

void hashmap_destroy(struct hashmap *htab)
{
    free(htab->tab);
}

void hashmap_apply(struct hashmap *htab, void (*mapper)(struct hashmap_item *ptr))
{
    for (size_t i = 0; i < htab->capacity; i++) {
        struct hashmap_item *next;
        for (struct hashmap_item *cur = htab->tab[i]; cur; cur = next) {
            next = cur->next;
            if (cur)
                mapper(cur);
        }
    }
}
