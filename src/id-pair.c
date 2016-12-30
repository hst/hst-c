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

#include "ccan/compiler/compiler.h"
#include "ccan/likely/likely.h"
#include "id-pair.h"

/*------------------------------------------------------------------------------
 * Arrays
 */

void
csp_id_pair_array_init(struct csp_id_pair_array *array)
{
    array->pairs = array->internal;
    array->allocated_count = CSP_ID_PAIR_ARRAY_INTERNAL_SIZE;
    array->count = 0;
}

void
csp_id_pair_array_done(struct csp_id_pair_array *array)
{
    if (array->pairs != array->internal) {
        free(array->pairs);
    }
}

#define CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT 32

void
csp_id_pair_array_ensure_size(struct csp_id_pair_array *array, size_t count)
{
    array->count = count;
    if (unlikely(count > array->allocated_count)) {
        if (array->pairs == array->internal) {
            size_t new_count = CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT;
            while (count > new_count) {
                new_count *= 2;
            }
            array->pairs = malloc(new_count * sizeof(struct csp_id_pair));
            assert(array->pairs != NULL);
            array->allocated_count = new_count;
        } else {
            /* Whenever we reallocate, at least double the size of the existing
             * array. */
            struct csp_id_pair *new_pairs;
            size_t new_count = array->allocated_count;
            do {
                new_count *= 2;
            } while (count > new_count);
            new_pairs = realloc(array->pairs,
                                new_count * sizeof(struct csp_id_pair));
            assert(new_pairs != NULL);
            array->pairs = new_pairs;
            array->allocated_count = new_count;
        }
    }
}

bool
csp_id_pair_array_eq(const struct csp_id_pair_array *a1,
                     const struct csp_id_pair_array *a2)
{
    if (a1->count != a2->count) {
        return false;
    }
    return memcmp(a1->pairs, a2->pairs,
                  a1->count * sizeof(struct csp_id_pair)) == 0;
}

/*------------------------------------------------------------------------------
 * Sets
 */

void
csp_id_pair_set_init(struct csp_id_pair_set *set)
{
    set->pairs = set->internal;
    set->allocated_count = CSP_ID_PAIR_SET_INTERNAL_SIZE;
    set->count = 0;
}

void
csp_id_pair_set_done(struct csp_id_pair_set *set)
{
    if (set->pairs != set->internal) {
        free(set->pairs);
    }
}

static int
csp_id_pair_cmp(const void *vp1, const void *vp2)
{
    const struct csp_id_pair *p1 = vp1;
    const struct csp_id_pair *p2 = vp2;
    /* This comparison ordering happens to line up with how we're storing pairs
     * in the set builder, which defines in which order the pairs are added to
     * the array when building the set. */

    if (p1->from < p2->from) {
        return -1;
    } else if (p1->from > p2->from) {
        return 1;
    }

    if (p1->to < p2->to) {
        return -1;
    } else if (p1->to > p2->to) {
        return 1;
    }

    return 0;
}

bool
csp_id_pair_set_contains(const struct csp_id_pair_set *set,
                         struct csp_id_pair pair)
{
    /* The pairs list is sorted, so we can do a binary search to determine if an
     * element is in the set. */
    void *vpair = bsearch(&pair, set->pairs, set->count,
                          sizeof(struct csp_id_pair), csp_id_pair_cmp);
    return vpair != NULL;
}

bool
csp_id_pair_set_eq(const struct csp_id_pair_set *set1,
                   const struct csp_id_pair_set *set2)
{
    if (set1->count != set2->count) {
        return false;
    }
    return memcmp(set1->pairs, set2->pairs,
                  set1->count * sizeof(struct csp_id_pair)) == 0;
}

void
csp_id_pair_set_builder_init(struct csp_id_pair_set_builder *builder)
{
    builder->count = 0;
    builder->working_set = NULL;
}

void
csp_id_pair_set_builder_done(struct csp_id_pair_set_builder *builder)
{
    {
        UNNEEDED Word_t dummy;
        Word_t *vtos;
        csp_id from = 0;
        JLF(vtos, builder->working_set, from);
        while (vtos != NULL) {
            void *tos = (void *) *vtos;
            J1FA(dummy, tos);
            JLN(vtos, builder->working_set, from);
        }
        JLFA(dummy, builder->working_set);
    }
}

void
csp_id_pair_set_builder_add(struct csp_id_pair_set_builder *builder,
                            struct csp_id_pair pair)
{
    int rc;
    Word_t *vtos;
    void **tos;
    JLI(vtos, builder->working_set, pair.from);
    tos = (void **) vtos;
    J1S(rc, *tos, pair.to);
    builder->count += rc;
}

#define CSP_ID_PAIR_SET_FIRST_ALLOCATION_COUNT 32

static void
csp_id_pair_set_ensure_size(struct csp_id_pair_set *set, size_t count)
{
    set->count = count;
    if (unlikely(count > set->allocated_count)) {
        if (set->pairs == set->internal) {
            size_t new_count = CSP_ID_PAIR_SET_FIRST_ALLOCATION_COUNT;
            while (count > new_count) {
                new_count *= 2;
            }
            set->pairs = malloc(new_count * sizeof(struct csp_id_pair));
            assert(set->pairs != NULL);
            set->allocated_count = new_count;
        } else {
            /* Whenever we reallocate, at least double the size of the existing
             * set. */
            struct csp_id_pair *new_pairs;
            size_t new_count = set->allocated_count;
            do {
                new_count *= 2;
            } while (count > new_count);
            new_pairs =
                    realloc(set->pairs, new_count * sizeof(struct csp_id_pair));
            assert(new_pairs != NULL);
            set->pairs = new_pairs;
            set->allocated_count = new_count;
        }
    }
}

void
csp_id_pair_set_build(struct csp_id_pair_set *set,
                      struct csp_id_pair_set_builder *builder)
{
    UNNEEDED Word_t dummy;
    size_t i = 0;
    Word_t *vtos;
    csp_id from = 0;

    csp_id_pair_set_ensure_size(set, builder->count);
    JLF(vtos, builder->working_set, from);
    while (vtos != NULL) {
        void **tos = (void **) vtos;
        int found;
        csp_id to = 0;
        J1F(found, *tos, to);
        while (found) {
            set->pairs[i].from = from;
            set->pairs[i].to = to;
            i++;
            J1N(found, *tos, to);
        }

        tos = (void **) vtos;
        J1FA(dummy, *tos);
        JLN(vtos, builder->working_set, from);
    }

    JLFA(dummy, builder->working_set);
    builder->count = 0;
}

#define swap_id_pair_set(a, b)         \
    do {                               \
        struct csp_id_pair_set __swap; \
        __swap = (a);                  \
        (a) = (b);                     \
        (b) = __swap;                  \
    } while (0)

void
csp_id_pair_set_union(struct csp_id_pair_set *set1,
                      const struct csp_id_pair_set *set2)
{
    size_t i1;
    size_t i2;
    size_t d;
    size_t duplicates = 0;
    struct csp_id_pair_set dest;

    /* Merge the two sorted list of pairs together, stopping when we run out of
     * pairs in one or both of the sources. */
    csp_id_pair_set_init(&dest);
    csp_id_pair_set_ensure_size(&dest, set1->count + set2->count);
    for (d = 0, i1 = 0, i2 = 0; i1 < set1->count && i2 < set2->count;) {
        const struct csp_id_pair *p1 = &set1->pairs[i1];
        const struct csp_id_pair *p2 = &set2->pairs[i2];
        int cmp = csp_id_pair_cmp(p1, p2);
        if (cmp < 0) {
            dest.pairs[d++] = *p1;
            i1++;
        } else {
            dest.pairs[d++] = *p2;
            i2++;
            if (cmp == 0) {
                i1++;
                duplicates++;
            }
        }
    }

    /* Copy the remainder of set1 or set2, if any, to the result. */
    for (; i1 < set1->count; i1++) {
        dest.pairs[d++] = set1->pairs[i1];
    }
    for (; i2 < set2->count; i2++) {
        dest.pairs[d++] = set2->pairs[i2];
    }

    /* If there were any pairs that were in both set1 and set2, then we
     * overestimated the size of dest when we first allocated space for its
     * pairs array. */
    dest.count = d;

    /* Swap the dest into the output parameter and free the old copy. */
    csp_id_pair_set_done(set1);
    swap_id_pair_set(dest, *set1);
    if (set1->pairs == dest.internal) {
        set1->pairs = set1->internal;
    }
}
