/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-set.h"

#include <stdbool.h>
#include <stdlib.h>

#include "set.h"

#define CSP_ID_SET_INITIAL_HASH UINT64_C(0x1866bb10b2b2fad7) /* random */

void
csp_id_set_init(struct csp_id_set *set)
{
    csp_set_init(&set->set);
}

void
csp_id_set_done(struct csp_id_set *set)
{
    csp_set_done(&set->set, NULL, NULL);
}

void
csp_id_set_clear(struct csp_id_set *set)
{
    csp_set_clear(&set->set, NULL, NULL);
}

csp_id
csp_id_set_hash(const struct csp_id_set *set)
{
    return csp_set_hash(&set->set, CSP_ID_SET_INITIAL_HASH);
}

bool
csp_id_set_empty(const struct csp_id_set *set)
{
    return csp_set_empty(&set->set);
}

size_t
csp_id_set_size(const struct csp_id_set *set)
{
    return csp_set_size(&set->set);
}

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2)
{
    return csp_set_eq(&set1->set, &set2->set);
}

bool
csp_id_set_subseteq(const struct csp_id_set *set1,
                    const struct csp_id_set *set2)
{
    return csp_set_subseteq(&set1->set, &set2->set);
}

bool
csp_id_set_add(struct csp_id_set *set, csp_id id)
{
    return csp_set_add(&set->set, (void *) (uintptr_t) id);
}

bool
csp_id_set_add_many(struct csp_id_set *set, size_t count, csp_id *ids)
{
    bool any_new = false;
    size_t i;
    for (i = 0; i < count; i++) {
        if (csp_id_set_add(set, ids[i])) {
            any_new = true;
        }
    }
    return any_new;
}

bool
csp_id_set_remove(struct csp_id_set *set, csp_id id)
{
    return csp_set_remove(&set->set, (void *) (uintptr_t) id);
}

bool
csp_id_set_remove_many(struct csp_id_set *set, size_t count, csp_id *ids)
{
    bool any_removed = false;
    size_t i;
    for (i = 0; i < count; i++) {
        if (csp_id_set_remove(set, ids[i])) {
            any_removed = true;
        }
    }
    return any_removed;
}

bool
csp_id_set_union(struct csp_id_set *set, const struct csp_id_set *other)
{
    return csp_set_union(&set->set, &other->set);
}

void
csp_id_set_get_iterator(const struct csp_id_set *set,
                        struct csp_id_set_iterator *iter)
{
    csp_set_get_iterator(&set->set, &iter->iter);
}

csp_id
csp_id_set_iterator_get(const struct csp_id_set_iterator *iter)
{
    return (uintptr_t) csp_set_iterator_get(&iter->iter);
}

bool
csp_id_set_iterator_done(struct csp_id_set_iterator *iter)
{
    return csp_set_iterator_done(&iter->iter);
}

void
csp_id_set_iterator_advance(struct csp_id_set_iterator *iter)
{
    csp_set_iterator_advance(&iter->iter);
}
