/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ID_MAP_H
#define HST_ID_MAP_H

#include "basics.h"
#include "map.h"

/* An ID→ID map.  Each ID is mapped to at most one other ID. */
struct csp_id_map {
    struct csp_map map;
};

void
csp_id_map_init(struct csp_id_map *map);

void
csp_id_map_done(struct csp_id_map *map);

bool
csp_id_map_eq(const struct csp_id_map *map1, const struct csp_id_map *map2);

size_t
csp_id_map_size(const struct csp_id_map *map);

/* Return the ID that `from` is mapped to. */
csp_id
csp_id_map_get(const struct csp_id_map *map, csp_id from);

/* Insert a new entry into an ID→ID map.  Overwrite any existing entry with the
 * same `from`.  Return the old `to` value, or CSP_ID_NONE if there wasn't
 * already an entry with this `from`. */
csp_id
csp_id_map_insert(struct csp_id_map *map, csp_id from, csp_id to);

struct csp_id_map_iterator {
    struct csp_map_iterator iter;
};

void
csp_id_map_get_iterator(const struct csp_id_map *map,
                        struct csp_id_map_iterator *iter);

bool
csp_id_map_iterator_done(struct csp_id_map_iterator *iter);

void
csp_id_map_iterator_advance(struct csp_id_map_iterator *iter);

csp_id
csp_id_map_iterator_get_from(const struct csp_id_map_iterator *iter);

csp_id
csp_id_map_iterator_get_to(const struct csp_id_map_iterator *iter);

#define csp_id_map_foreach(map, iter)            \
    for (csp_id_map_get_iterator((map), (iter)); \
         !csp_id_map_iterator_done((iter));      \
         csp_id_map_iterator_advance((iter)))

#endif /* HST_ID_MAP_H */
