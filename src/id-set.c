/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/build_assert/build_assert.h"
#include "ccan/compiler/compiler.h"
#include "ccan/likely/likely.h"
#include "hst.h"

/* Hashing sets: We use Zobrist hashes to maintain a hash for each set as we
 * build it.  A Zobrist hash relies on each possible element having a distinct
 * (and uniformly distributed) hash value, which is true of our process and
 * event IDs.  You then simply XOR together the hashes of each element (possibly
 * along with some initial base value) to get the hash of the set. */

#define CSP_ID_SET_INITIAL_HASH UINT64_C(0x1866bb10b2b2fad7) /* random */

void
csp_id_set_init(struct csp_id_set *set)
{
    set->hash = CSP_ID_SET_INITIAL_HASH;
    set->count = 0;
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

void
csp_id_set_iterate(const struct csp_id_set *set,
                   struct csp_id_set_iterator *iter)
{
    iter->elements = &set->elements;
    iter->current = 0;
    J1F(iter->found, *iter->elements, iter->current);
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

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2)
{
    if (set1->count != set2->count) {
        return false;
    } else if (set1->hash != set2->hash) {
        return false;
    } else {
        struct csp_id_set_iterator iter1;
        struct csp_id_set_iterator iter2;
        /* Loop through the elements in `set1`, verifying that each one is also
         * in `set2`. */
        for (csp_id_set_iterate(set1, &iter1), csp_id_set_iterate(set2, &iter2);
             !csp_id_set_iterator_done(&iter1) &&
             !csp_id_set_iterator_done(&iter2);
             csp_id_set_iterator_advance(&iter1),
             csp_id_set_iterator_advance(&iter2)) {
            if (iter1.current != iter2.current) {
                return false;
            }
        }
        return csp_id_set_iterator_done(&iter1) ==
               csp_id_set_iterator_done(&iter2);
    }
}

bool
csp_id_set_subseteq(const struct csp_id_set *set1,
                    const struct csp_id_set *set2)
{
    struct csp_id_set_iterator iter1;
    struct csp_id_set_iterator iter2;
    /* Loop through the elements in `set1`, verifying that each one is also in
     * `set2`. */
    for (csp_id_set_iterate(set1, &iter1), csp_id_set_iterate(set2, &iter2);
         !csp_id_set_iterator_done(&iter1);
         csp_id_set_iterator_advance(&iter1),
         csp_id_set_iterator_advance(&iter2)) {
        while (!csp_id_set_iterator_done(&iter2) &&
               iter1.current > iter2.current) {
            csp_id_set_iterator_advance(&iter2);
        }
        if (csp_id_set_iterator_done(&iter2) || iter1.current < iter2.current) {
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
    if (rc) {
        /* Only update the set's hash and count if we actually added a new
         * element. */
        set->hash ^= id;
        set->count++;
    }
    return rc;
}

static bool
csp_id_set_remove_one(struct csp_id_set *set, csp_id id)
{
    int rc;
    J1U(rc, set->elements, id);
    if (rc) {
        /* Only update the set's hash and count if we actually removed an
         * element. */
        set->hash ^= id;
        set->count--;
    }
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
        if (csp_id_set_add_one(set, iter.current)) {
            any_new = true;
        }
    }
    return any_new;
}
