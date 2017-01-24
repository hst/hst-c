/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/compiler/compiler.h"
#include "ccan/likely/likely.h"
#include "id-pair.h"

/*------------------------------------------------------------------------------
 * Sets
 */

static int
csp_id_pair_cmp(const void *vp1, const void *vp2)
{
    const struct csp_id_pair *p1 = vp1;
    const struct csp_id_pair *p2 = vp2;

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

void
csp_id_pair_set_init(struct csp_id_pair_set *set)
{
    set->avl = avl_new(csp_id_pair_cmp);
    assert(set->avl != NULL);
}

void
csp_id_pair_set_done(struct csp_id_pair_set *set)
{
    struct csp_id_pair_set_iterator iter;
    csp_id_pair_set_foreach (set, &iter) {
        const struct csp_id_pair *pair = csp_id_pair_set_iterator_get(&iter);
        free((void *) pair);
    }
    avl_free(set->avl);
}

void
csp_id_pair_set_clear(struct csp_id_pair_set *set)
{
    csp_id_pair_set_done(set);
    csp_id_pair_set_init(set);
}

bool
csp_id_pair_set_empty(const struct csp_id_pair_set *set)
{
    return avl_count(set->avl) == 0;
}

size_t
csp_id_pair_set_size(const struct csp_id_pair_set *set)
{
    return avl_count(set->avl);
}

void
csp_id_pair_set_add(struct csp_id_pair_set *set, struct csp_id_pair pair)
{
    if (avl_lookup(set->avl, &pair) == NULL) {
        struct csp_id_pair *pair_copy = malloc(sizeof(struct csp_id_pair));
        *pair_copy = pair;
        avl_insert(set->avl, pair_copy, pair_copy);
    }
}

bool
csp_id_pair_set_contains(const struct csp_id_pair_set *set,
                         struct csp_id_pair pair)
{
    return avl_lookup(set->avl, &pair) != NULL;
}

void
csp_id_pair_set_union(struct csp_id_pair_set *set1,
                      const struct csp_id_pair_set *set2)
{
    struct csp_id_pair_set_iterator iter;
    csp_id_pair_set_foreach (set2, &iter) {
        const struct csp_id_pair *pair = csp_id_pair_set_iterator_get(&iter);
        csp_id_pair_set_add(set1, *pair);
    }
}

void
csp_id_pair_set_get_iterator(const struct csp_id_pair_set *set,
                             struct csp_id_pair_set_iterator *iter)
{
    avl_iter_begin(&iter->iter, set->avl, FORWARD);
}

const struct csp_id_pair *
csp_id_pair_set_iterator_get(const struct csp_id_pair_set_iterator *iter)
{
    return iter->iter.key;
}

bool
csp_id_pair_set_iterator_done(struct csp_id_pair_set_iterator *iter)
{
    return iter->iter.node == NULL;
}

void
csp_id_pair_set_iterator_advance(struct csp_id_pair_set_iterator *iter)
{
    avl_iter_next(&iter->iter);
}
