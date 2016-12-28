/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ID_SET_MAP_H
#define HST_ID_SET_MAP_H

#include "id-set.h"
#include "map.h"

/* ID→{ID} map */
struct csp_id_set_map {
    struct csp_map map;
};

void
csp_id_set_map_init(struct csp_id_set_map *map);

void
csp_id_set_map_done(struct csp_id_set_map *map);

bool
csp_id_set_map_eq(const struct csp_id_set_map *map1,
                  const struct csp_id_set_map *map2);

const struct csp_id_set *
csp_id_set_map_get(const struct csp_id_set_map *map, csp_id id);

/* Return whether there was already an entry with the same `from` and `to`. */
bool
csp_id_set_map_insert(struct csp_id_set_map *map, csp_id from, csp_id to);

/* Return whether we removed an entry with this `from` and `to`. */
bool
csp_id_set_map_remove(struct csp_id_set_map *map, csp_id from, csp_id to);

struct csp_id_set_map_iterator {
    struct csp_map_iterator iter;
};

void
csp_id_set_map_get_iterator(const struct csp_id_set_map *map,
                            struct csp_id_set_map_iterator *iter);

bool
csp_id_set_map_iterator_done(struct csp_id_set_map_iterator *iter);

void
csp_id_set_map_iterator_advance(struct csp_id_set_map_iterator *iter);

csp_id
csp_id_set_map_iterator_get_from(const struct csp_id_set_map_iterator *iter);

const struct csp_id_set *
csp_id_set_map_iterator_get_tos(const struct csp_id_set_map_iterator *iter);

#define csp_id_set_map_foreach(map, iter)            \
    for (csp_id_set_map_get_iterator((map), (iter)); \
         !csp_id_set_map_iterator_done((iter));      \
         csp_id_set_map_iterator_advance((iter)))

#endif /* HST_ID_SET_MAP_H */
