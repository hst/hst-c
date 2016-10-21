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

void
csp_id_set_init(struct csp_id_set *set)
{
    set->ids = set->internal;
    set->allocated_count = CSP_ID_SET_INTERNAL_SIZE;
}

void
csp_id_set_done(struct csp_id_set *set)
{
    if (set->ids != set->internal) {
        free(set->ids);
    }
}

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2)
{
    if (set1->count != set2->count) {
        return false;
    } else {
        return memcmp(set1->ids, set2->ids, set1->count * sizeof(csp_id)) == 0;
    }
}

void
csp_id_set_builder_init(struct csp_id_set_builder *builder)
{
    builder->working_set = NULL;
}

void
csp_id_set_builder_done(struct csp_id_set_builder *builder)
{
    UNNEEDED Word_t  dummy;
    J1FA(dummy, builder->working_set);
}

void
csp_id_set_builder_add(struct csp_id_set_builder *builder, csp_id id)
{
    UNNEEDED int  rc;
    J1S(rc, builder->working_set, id);
}

void
csp_id_set_builder_add_many(struct csp_id_set_builder *builder, size_t count,
                            csp_id *ids)
{
    UNNEEDED int  rc;
    size_t  i;
    for (i = 0; i < count; i++) {
        J1S(rc, builder->working_set, ids[i]);
    }
}

bool
csp_id_set_builder_remove(struct csp_id_set_builder *builder, csp_id id)
{
    int  rc;
    J1U(rc, builder->working_set, id);
    return rc;
}

void
csp_id_set_builder_remove_many(struct csp_id_set_builder *builder, size_t count,
                               csp_id *ids)
{
    UNNEEDED int  rc;
    size_t  i;
    for (i = 0; i < count; i++) {
        J1U(rc, builder->working_set, ids[i]);
    }
}

void
csp_id_set_builder_merge(struct csp_id_set_builder *builder,
                         const struct csp_id_set *set)
{
    UNNEEDED int  rc;
    size_t  i;
    for (i = 0; i < set->count; i++) {
        J1S(rc, builder->working_set, set->ids[i]);
    }
}

#define CSP_ID_SET_FIRST_ALLOCATION_COUNT  32

/* Ensure that `set` is large enough to hold `set->count` elements. */
static void
csp_id_set_ensure_size(struct csp_id_set *set)
{
    if (unlikely(set->count > set->allocated_count)) {
        if (set->ids == set->internal) {
            size_t  new_count = CSP_ID_SET_FIRST_ALLOCATION_COUNT;
            while (set->count > new_count) {
                new_count *= 2;
            }
            set->ids = malloc(new_count * sizeof(csp_id));
            assert(set->ids != NULL);
            set->allocated_count = CSP_ID_SET_FIRST_ALLOCATION_COUNT;
        } else {
            /* Whenever we reallocate, at least double the size of the existing
             * array. */
            csp_id  *new_ids;
            size_t  new_count = set->allocated_count;
            do {
                new_count *= 2;
            } while (set->count > new_count);
            new_ids = realloc(set->ids, new_count * sizeof(csp_id));
            assert(new_ids != NULL);
            set->ids = new_ids;
            set->allocated_count = new_count;
        }
    }
}

void
csp_id_set_build_and_keep(struct csp_id_set *set,
                          struct csp_id_set_builder *builder)
{
    int  found;
    size_t  i;
    csp_id  id;

    /* First make sure that the `ids` array is large enough to hold all of the
     * ids that have been added to the set. */
    J1C(set->count, builder->working_set, 0, -1);
    csp_id_set_ensure_size(set);

    /* Then fill in the array. */
    i = 0;
    id = 0;
    J1F(found, builder->working_set, id);
    while (found) {
        set->ids[i++] = id;
        J1N(found, builder->working_set, id);
    }
}

void
csp_id_set_build(struct csp_id_set *set, struct csp_id_set_builder *builder)
{
    UNNEEDED Word_t  dummy;
    csp_id_set_build_and_keep(set, builder);
    J1FA(dummy, builder->working_set);
}

void
csp_id_set_clone(struct csp_id_set *set, const struct csp_id_set *other)
{
    set->count = other->count;
    csp_id_set_ensure_size(set);
    memcpy(set->ids, other->ids, other->count * sizeof(csp_id));
}

void
csp_id_set_fill_single(struct csp_id_set *set, csp_id event)
{
    /* Ensure that we can initialize the singleton set without having to
     * allocate anything.  Because we always reallocate some space in the set
     * itself, we can do this at build time. */
    BUILD_ASSERT(CSP_ID_SET_INTERNAL_SIZE >= 1);
    set->count = 1;
    set->ids[0] = event;
}

void
csp_id_set_fill_double(struct csp_id_set *set, csp_id e1, csp_id e2)
{
    /* Ensure that we can initialize the singleton set without having to
     * allocate anything.  Because we always reallocate some space in the set
     * itself, we can do this at build time. */
    BUILD_ASSERT(CSP_ID_SET_INTERNAL_SIZE >= 2);

    /* Make sure the events are deduplicated! */
    if (unlikely(e1 == e2)) {
        set->count = 1;
        set->ids[0] = e1;
    } else {
        set->count = 2;
        /* Make sure the events are sorted! */
        if (e1 < e2) {
            set->ids[0] = e1;
            set->ids[1] = e2;
        } else {
            set->ids[0] = e2;
            set->ids[1] = e1;
        }
    }
}
