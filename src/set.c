/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "set.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"
#include "ccan/hash/hash.h"

/* Hashing sets: We use Zobrist hashes to calculate a hash for each set.  A
 * Zobrist hash relies on each possible element having a distinct (and uniformly
 * distributed) hash value.  You then simply XOR together the hashes of each
 * element (possibly along with some initial base value) to get the hash of the
 * set. */

#define CSP_SET_INITIAL_HASH UINT64_C(0xec87ea715d6826f5) /* random */

void
csp_set_init(struct csp_set *set)
{
    set->elements = NULL;
}

void
csp_set_done(struct csp_set *set, csp_set_free_entry_f *free_entry, void *ud)
{
    UNNEEDED Word_t dummy;
    if (free_entry != NULL) {
        struct csp_set_iterator iter;
        csp_set_foreach (set, &iter) {
            void *entry = csp_set_iterator_get(&iter);
            free_entry(ud, entry);
        }
    }
    J1FA(dummy, set->elements);
}

void
csp_set_clear(struct csp_set *set, csp_set_free_entry_f *free_entry, void *ud)
{
    csp_set_done(set, free_entry, ud);
    csp_set_init(set);
}

uint64_t
csp_set_hash(const struct csp_set *set, uint64_t base)
{
    struct csp_set_iterator iter;
    uint64_t hash = CSP_SET_INITIAL_HASH;
    csp_set_foreach (set, &iter) {
        void *element = csp_set_iterator_get(&iter);
        uint64_t element_hash = hash64_any(&element, sizeof(element), base);
        hash ^= element_hash;
    }
    return hash;
}

bool
csp_set_empty(const struct csp_set *set)
{
    return set->elements == NULL;
}

size_t
csp_set_size(const struct csp_set *set)
{
    Word_t count;
    J1C(count, set->elements, 0, -1);
    return count;
}

bool
csp_set_eq(const struct csp_set *set1, const struct csp_set *set2)
{
    struct csp_set_iterator iter1;
    struct csp_set_iterator iter2;
    /* Loop through the elements in `set1`, verifying that each one is also
     * in `set2`. */
    for (csp_set_get_iterator(set1, &iter1), csp_set_get_iterator(set2, &iter2);
         !csp_set_iterator_done(&iter1) && !csp_set_iterator_done(&iter2);
         csp_set_iterator_advance(&iter1), csp_set_iterator_advance(&iter2)) {
        if (csp_set_iterator_get(&iter1) != csp_set_iterator_get(&iter2)) {
            return false;
        }
    }
    return csp_set_iterator_done(&iter1) == csp_set_iterator_done(&iter2);
}

bool
csp_set_subseteq(const struct csp_set *set1, const struct csp_set *set2)
{
    struct csp_set_iterator iter1;
    struct csp_set_iterator iter2;
    /* Loop through the elements in `set1`, verifying that each one is also in
     * `set2`. */
    for (csp_set_get_iterator(set1, &iter1), csp_set_get_iterator(set2, &iter2);
         !csp_set_iterator_done(&iter1);
         csp_set_iterator_advance(&iter1), csp_set_iterator_advance(&iter2)) {
        while (!csp_set_iterator_done(&iter2) &&
               csp_set_iterator_get(&iter1) > csp_set_iterator_get(&iter2)) {
            csp_set_iterator_advance(&iter2);
        }
        if (csp_set_iterator_done(&iter2) ||
            csp_set_iterator_get(&iter1) < csp_set_iterator_get(&iter2)) {
            return false;
        }
    }
    return true;
}

bool
csp_set_add(struct csp_set *set, void *element)
{
    int rc;
    J1S(rc, set->elements, (uintptr_t) element);
    return rc;
}

bool
csp_set_add_many(struct csp_set *set, size_t count, void **elements)
{
    bool any_new = false;
    size_t i;
    for (i = 0; i < count; i++) {
        if (csp_set_add(set, elements[i])) {
            any_new = true;
        }
    }
    return any_new;
}

bool
csp_set_remove(struct csp_set *set, void *element)
{
    int rc;
    J1U(rc, set->elements, (uintptr_t) element);
    return rc;
}

bool
csp_set_remove_many(struct csp_set *set, size_t count, void **elements)
{
    bool any_removed = false;
    size_t i;
    for (i = 0; i < count; i++) {
        if (csp_set_remove(set, elements[i])) {
            any_removed = true;
        }
    }
    return any_removed;
}

bool
csp_set_union(struct csp_set *set, const struct csp_set *other)
{
    bool any_new = false;
    struct csp_set_iterator iter;
    csp_set_foreach (other, &iter) {
        if (csp_set_add(set, csp_set_iterator_get(&iter))) {
            any_new = true;
        }
    }
    return any_new;
}

void
csp_set_get_iterator(const struct csp_set *set, struct csp_set_iterator *iter)
{
    iter->elements = &set->elements;
    iter->current = 0;
    J1F(iter->found, *iter->elements, iter->current);
}

void *
csp_set_iterator_get(const struct csp_set_iterator *iter)
{
    return (void *) iter->current;
}

bool
csp_set_iterator_done(struct csp_set_iterator *iter)
{
    return !iter->found;
}

void
csp_set_iterator_advance(struct csp_set_iterator *iter)
{
    J1N(iter->found, *iter->elements, iter->current);
}
