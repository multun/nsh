#pragma once

#include <assert.h>
#include <stdbool.h>

#include "utils/macros.h"


struct list_head
{
    struct list_head *prev;
    struct list_head *next;
};

static inline bool list_empty(struct list_head *head)
{
    return head->next == head && head->prev == head;
}

static inline bool list_single(struct list_head *head)
{
    return head->next != head && head->next == head->prev;
}

static inline void list_init(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    struct list_head *prev = head;
    struct list_head *next = head->next;
    next->prev = (new);
    (new)->next = next;
    (new)->prev = prev;
    prev->next = (new);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    struct list_head *prev = head->prev;
    struct list_head *next = head;
    next->prev = (new);
    (new)->next = next;
    (new)->prev = prev;
    prev->next = (new);
}

static inline void list_del(struct list_head *list)
{
    struct list_head *next = (list)->next;
    struct list_head *prev = (list)->prev;
    next->prev = prev;
    prev->next = next;
#ifndef NDEBUG
    list->next = NULL;
    list->prev = NULL;
#endif
}

#define list_for_each_entry(pos, head, type, member)            \
    for (pos = container_of((head)->next, type, member);        \
         &pos->member != (head);                                \
         pos = container_of(pos->member.next, type, member))

#define list_for_each_entry_safe(pos, tmp, head, type, member)  \
    for (pos = container_of((head)->next, type, member),        \
         tmp = container_of(pos->member.next, type, member);    \
         &pos->member != (head);                                \
         pos = tmp,                                             \
         tmp = container_of(tmp->member.next, type, member))
