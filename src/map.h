/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_MAP_H
#define HST_MAP_H

#include <stdbool.h>
#include <stdlib.h>

#include "basics.h"

/* A map whose keys are IDs, and whose values are anything pointer-sized that
 * you want.  You'll typically never use this type directly; instead, there are
 * several helper types that store particular kinds of values, and you'll use
 * one of those directly.  (Take a look at csp_id_map to see an example of how
 * to implement one of those more specialized map types.) */
struct csp_map {
    void *entries;
};

void
csp_map_init(struct csp_map *map);

typedef void
csp_map_free_entry_f(void *ud, void *entry);

void
csp_map_done(struct csp_map *map, csp_map_free_entry_f *free_entry, void *ud);

typedef bool
csp_map_entry_eq_f(void *ud, const void *entry1, const void *entry2);

bool
csp_map_eq(const struct csp_map *map1, const struct csp_map *map2,
           csp_map_entry_eq_f *entry_eq, void *ud);

size_t
csp_map_size(const struct csp_map *map);

/* Return NULL if the entry doesn't exist. */
void *
csp_map_get(const struct csp_map *map, csp_id id);

/* Creates an entry if it doesn't exist, initialized to NULL. */
void **
csp_map_at(struct csp_map *map, csp_id id);

typedef void
csp_map_init_entry_f(void *ud, void **entry);

/* Ensure that `map` contains an entry with the given `id`, creating it if
 * necessary.  If the entry is new, calls `init_entry` to initialize it.  Return
 * the entry. */
void *
csp_map_insert(struct csp_map *map, csp_id id, csp_map_init_entry_f *init_entry,
               void *ud);

struct csp_map_iterator {
    void *const *entries;
    csp_id key;
    void **value;
};

void
csp_map_get_iterator(const struct csp_map *map, struct csp_map_iterator *iter);

bool
csp_map_iterator_done(struct csp_map_iterator *iter);

void
csp_map_iterator_advance(struct csp_map_iterator *iter);

#define csp_map_foreach(map, iter)                                            \
    for (csp_map_get_iterator((map), (iter)); !csp_map_iterator_done((iter)); \
         csp_map_iterator_advance((iter)))

#endif /* HST_MAP_H */
