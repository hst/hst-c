/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ID_SET_H
#define HST_ID_SET_H

#include <stdbool.h>
#include <stdlib.h>

#include "basics.h"

/* A set of IDs */
struct csp_id_set {
    csp_id hash;
    size_t count;
    void *elements;
};

void
csp_id_set_init(struct csp_id_set *set);

void
csp_id_set_done(struct csp_id_set *set);

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2);

/* Return whether set1 ⊆ set2 */
bool
csp_id_set_subseteq(const struct csp_id_set *set1,
                    const struct csp_id_set *set2);

void
csp_id_set_clear(struct csp_id_set *set);

/* Add a single ID to a set.  Return whether the ID is new (i.e., it wasn't
 * already in `set`.) */
bool
csp_id_set_add(struct csp_id_set *set, csp_id id);

/* Add several IDs to a set.  `ids` does not need to be sorted, and it's okay
 * for it to contain duplicates.  Returns true if any of the elements were not
 * already in the set. */
bool
csp_id_set_add_many(struct csp_id_set *set, size_t count, csp_id *ids);

/* Remove a single ID from a set.  Returns whether that ID was in the set or
 * not. */
bool
csp_id_set_remove(struct csp_id_set *set, csp_id id);

/* Remove several IDs from a set.  `ids` does not need to be sorted, and it's
 * okay for it to contain duplicates.  Returns true if any of elements were
 * removed. */
bool
csp_id_set_remove_many(struct csp_id_set *set, size_t count, csp_id *ids);

/* Add the contents of an existing set to a set.  Returns true if any new
 * elements were added. */
bool
csp_id_set_union(struct csp_id_set *set, const struct csp_id_set *other);

struct csp_id_set_iterator {
    void *const *elements;
    csp_id current;
    int found;
};

void
csp_id_set_get_iterator(const struct csp_id_set *set,
                        struct csp_id_set_iterator *iter);

csp_id
csp_id_set_iterator_get(const struct csp_id_set_iterator *iter);

bool
csp_id_set_iterator_done(struct csp_id_set_iterator *iter);

void
csp_id_set_iterator_advance(struct csp_id_set_iterator *iter);

#define csp_id_set_foreach(set, iter)            \
    for (csp_id_set_get_iterator((set), (iter)); \
         !csp_id_set_iterator_done((iter));      \
         csp_id_set_iterator_advance((iter)))

#endif /* HST_ID_SET_H */
