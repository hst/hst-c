/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-set.h"

#include <stdbool.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"

/* Hashing sets: We use Zobrist hashes to maintain a hash for each set as we
 * build it.  A Zobrist hash relies on each possible element having a distinct
 * (and uniformly distributed) hash value, which is true of our process and
 * event IDs.  You then simply XOR together the hashes of each element (possibly
 * along with some initial base value) to get the hash of the set. */

#define CSP_ID_SET_INITIAL_HASH UINT64_C(0x1866bb10b2b2fad7) /* random */

void
csp_id_set_init(struct csp_id_set *set)
{
    set->elements = NULL;
}

void
csp_id_set_done(struct csp_id_set *set)
{
    UNNEEDED Word_t dummy;
    J1FA(dummy, set->elements);
}

void
csp_id_set_clear(struct csp_id_set *set)
{
    csp_id_set_done(set);
    csp_id_set_init(set);
}

csp_id
csp_id_set_hash(const struct csp_id_set *set)
{
    struct csp_id_set_iterator iter;
    csp_id hash = CSP_ID_SET_INITIAL_HASH;
    csp_id_set_foreach (set, &iter) {
        hash ^= csp_id_set_iterator_get(&iter);
    }
    return hash;
}

bool
csp_id_set_empty(const struct csp_id_set *set)
{
    return set->elements == NULL;
}

size_t
csp_id_set_size(const struct csp_id_set *set)
{
    Word_t count;
    J1C(count, set->elements, 0, -1);
    return count;
}

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2)
{
    struct csp_id_set_iterator iter1;
    struct csp_id_set_iterator iter2;
    /* Loop through the elements in `set1`, verifying that each one is also
     * in `set2`. */
    for (csp_id_set_get_iterator(set1, &iter1),
         csp_id_set_get_iterator(set2, &iter2);
         !csp_id_set_iterator_done(&iter1) && !csp_id_set_iterator_done(&iter2);
         csp_id_set_iterator_advance(&iter1),
         csp_id_set_iterator_advance(&iter2)) {
        if (csp_id_set_iterator_get(&iter1) !=
            csp_id_set_iterator_get(&iter2)) {
            return false;
        }
    }
    return csp_id_set_iterator_done(&iter1) == csp_id_set_iterator_done(&iter2);
}

bool
csp_id_set_subseteq(const struct csp_id_set *set1,
                    const struct csp_id_set *set2)
{
    struct csp_id_set_iterator iter1;
    struct csp_id_set_iterator iter2;
    /* Loop through the elements in `set1`, verifying that each one is also in
     * `set2`. */
    for (csp_id_set_get_iterator(set1, &iter1),
         csp_id_set_get_iterator(set2, &iter2);
         !csp_id_set_iterator_done(&iter1);
         csp_id_set_iterator_advance(&iter1),
         csp_id_set_iterator_advance(&iter2)) {
        while (!csp_id_set_iterator_done(&iter2) &&
               csp_id_set_iterator_get(&iter1) >
                       csp_id_set_iterator_get(&iter2)) {
            csp_id_set_iterator_advance(&iter2);
        }
        if (csp_id_set_iterator_done(&iter2) ||
            csp_id_set_iterator_get(&iter1) < csp_id_set_iterator_get(&iter2)) {
            return false;
        }
    }
    return true;
}

static bool
csp_id_set_add_one(struct csp_id_set *set, csp_id id)
{
    int rc;
    J1S(rc, set->elements, id);
    return rc;
}

static bool
csp_id_set_remove_one(struct csp_id_set *set, csp_id id)
{
    int rc;
    J1U(rc, set->elements, id);
    return rc;
}

bool
csp_id_set_add(struct csp_id_set *set, csp_id id)
{
    return csp_id_set_add_one(set, id);
}

bool
csp_id_set_add_many(struct csp_id_set *set, size_t count, csp_id *ids)
{
    bool any_new = false;
    size_t i;
    for (i = 0; i < count; i++) {
        if (csp_id_set_add_one(set, ids[i])) {
            any_new = true;
        }
    }
    return any_new;
}

bool
csp_id_set_remove(struct csp_id_set *set, csp_id id)
{
    return csp_id_set_remove_one(set, id);
}

bool
csp_id_set_remove_many(struct csp_id_set *set, size_t count, csp_id *ids)
{
    bool any_removed = false;
    size_t i;
    for (i = 0; i < count; i++) {
        if (csp_id_set_remove_one(set, ids[i])) {
            any_removed = true;
        }
    }
    return any_removed;
}

bool
csp_id_set_union(struct csp_id_set *set, const struct csp_id_set *other)
{
    bool any_new = false;
    struct csp_id_set_iterator iter;
    csp_id_set_foreach (other, &iter) {
        if (csp_id_set_add_one(set, csp_id_set_iterator_get(&iter))) {
            any_new = true;
        }
    }
    return any_new;
}

void
csp_id_set_get_iterator(const struct csp_id_set *set,
                        struct csp_id_set_iterator *iter)
{
    iter->elements = &set->elements;
    iter->current = 0;
    J1F(iter->found, *iter->elements, iter->current);
}

csp_id
csp_id_set_iterator_get(const struct csp_id_set_iterator *iter)
{
    return iter->current;
}

bool
csp_id_set_iterator_done(struct csp_id_set_iterator *iter)
{
    return !iter->found;
}

void
csp_id_set_iterator_advance(struct csp_id_set_iterator *iter)
{
    J1N(iter->found, *iter->elements, iter->current);
}
