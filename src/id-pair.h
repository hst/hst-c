/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ID_PAIR_H
#define HST_ID_PAIR_H

#include <stdbool.h>
#include <stdlib.h>

#include "ccan/avl/avl.h"
#include "basics.h"

/*------------------------------------------------------------------------------
 * Pairs
 */

struct csp_id_pair {
    csp_id from;
    csp_id to;
};

/* A set of pairs of IDs. */
struct csp_id_pair_set {
    AVL *avl;
};

void
csp_id_pair_set_init(struct csp_id_pair_set *set);

void
csp_id_pair_set_done(struct csp_id_pair_set *set);

void
csp_id_pair_set_clear(struct csp_id_pair_set *set);

bool
csp_id_pair_set_empty(const struct csp_id_pair_set *set);

size_t
csp_id_pair_set_size(const struct csp_id_pair_set *set);

void
csp_id_pair_set_add(struct csp_id_pair_set *set, struct csp_id_pair pair);

bool
csp_id_pair_set_contains(const struct csp_id_pair_set *set,
                         struct csp_id_pair pair);

/* Update `dest` to contain the union of `dest` and `src`. */
void
csp_id_pair_set_union(struct csp_id_pair_set *dest,
                      const struct csp_id_pair_set *src);

struct csp_id_pair_set_iterator {
    AvlIter iter;
};

void
csp_id_pair_set_get_iterator(const struct csp_id_pair_set *set,
                             struct csp_id_pair_set_iterator *iter);

const struct csp_id_pair *
csp_id_pair_set_iterator_get(const struct csp_id_pair_set_iterator *iter);

bool
csp_id_pair_set_iterator_done(struct csp_id_pair_set_iterator *iter);

void
csp_id_pair_set_iterator_advance(struct csp_id_pair_set_iterator *iter);

#define csp_id_pair_set_foreach(set, iter)            \
    for (csp_id_pair_set_get_iterator((set), (iter)); \
         !csp_id_pair_set_iterator_done((iter));      \
         csp_id_pair_set_iterator_advance((iter)))

#endif /* HST_ID_PAIR_H */
