/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_SET_H
#define HST_SET_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* A set of any pointer that you want.  The pointers must be directly comparable
 * — i.e., a pointer is only equal to itself; there isn't any way to define a
 * deeper comparison operation for your elements.  (If you want to do that, use
 * map.h to define a hash set.)  You'll typically never use this type directly;
 * instead, there are several helper types that store particular kinds of
 * elements, and you'll use one of those directly. */
struct csp_set {
    void *elements;
};

void
csp_set_init(struct csp_set *set);

typedef void
csp_set_free_entry_f(void *ud, void *entry);

void
csp_set_done(struct csp_set *set, csp_set_free_entry_f *free_entry, void *ud);

void
csp_set_clear(struct csp_set *set, csp_set_free_entry_f *free_entry, void *ud);

uint64_t
csp_set_hash(const struct csp_set *set, uint64_t base);

bool
csp_set_empty(const struct csp_set *set);

size_t
csp_set_size(const struct csp_set *set);

bool
csp_set_eq(const struct csp_set *set1, const struct csp_set *set2);

/* Return whether set1 ⊆ set2 */
bool
csp_set_subseteq(const struct csp_set *set1, const struct csp_set *set2);

/* Add a single element to a set.  Return whether the element is new (i.e., it
 * wasn't already in `set`.) */
bool
csp_set_add(struct csp_set *set, void *element);

/* Add several elements to a set.  Returns true if any of the elements were not
 * already in the set. */
bool
csp_set_add_many(struct csp_set *set, size_t count, void **elements);

/* Remove a single element from a set.  Returns whether that element was in the
 * set or not. */
bool
csp_set_remove(struct csp_set *set, void *element);

/* Remove several element from a set.  Returns true if any of elements were
 * removed. */
bool
csp_set_remove_many(struct csp_set *set, size_t count, void **elements);

/* Add the contents of an existing set to a set.  Returns true if any new
 * elements were added. */
bool
csp_set_union(struct csp_set *set, const struct csp_set *other);

struct csp_set_iterator {
    void *const *elements;
    uintptr_t current;
    int found;
};

void
csp_set_get_iterator(const struct csp_set *set, struct csp_set_iterator *iter);

void *
csp_set_iterator_get(const struct csp_set_iterator *iter);

bool
csp_set_iterator_done(struct csp_set_iterator *iter);

void
csp_set_iterator_advance(struct csp_set_iterator *iter);

#define csp_set_foreach(set, iter)            \
    for (csp_set_get_iterator((set), (iter)); \
         !csp_set_iterator_done((iter));      \
         csp_set_iterator_advance((iter)))

#endif /* HST_SET_H */
