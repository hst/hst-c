/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <errno.h>
#include <stdlib.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"
#include "ccan/likely/likely.h"
#include "hst.h"

void
csp_id_set_init(struct csp_id_set *set)
{
    set->ids = NULL;
    set->allocated_count = 0;
}

void
csp_id_set_done(struct csp_id_set *set)
{
    if (set->ids != NULL) {
        free(set->ids);
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

int
csp_id_set_build(struct csp_id_set *set, struct csp_id_set_builder *builder)
{
    int  found;
    size_t  i;
    csp_id  id;
    UNNEEDED Word_t  dummy;

    /* First make sure that the `ids` array is large enough to hold all of the
     * ids that have been added to the set. */
    J1C(set->count, builder->working_set, 0, -1);
    if (unlikely(set->count > set->allocated_count)) {
        /* Whenever we reallocate, at least double the size of the existing
         * array. */
        csp_id  *new_ids;
        size_t  new_count = set->allocated_count * 2;
        if (set->count > new_count) {
            new_count = set->count;
        }
        new_ids = realloc(set->ids, new_count * sizeof(csp_id));
        if (unlikely(new_ids == NULL)) {
            return ENOMEM;
        }
        set->ids = new_ids;
        set->allocated_count = new_count;
    }

    /* Then fill in the array. */
    i = 0;
    id = 0;
    J1F(found, builder->working_set, id);
    while (found) {
        set->ids[i++] = id;
        J1N(found, builder->working_set, id);
    }

    J1FA(dummy, builder->working_set);
    return 0;
}
