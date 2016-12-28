/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_STRING_MAP_H
#define HST_STRING_MAP_H

#include "map.h"

/* ID→string map */
struct csp_string_map {
    struct csp_map map;
};

void
csp_string_map_init(struct csp_string_map *map);

void
csp_string_map_done(struct csp_string_map *map);

bool
csp_string_map_eq(const struct csp_string_map *map1,
                  const struct csp_string_map *map2);

const char *
csp_string_map_get(const struct csp_string_map *map, csp_id id);

/* There must not already be an entry with the given key.  We create our own
 * copy of `str`. */
void
csp_string_map_insert(struct csp_string_map *map, csp_id id, const char *str);

/* There must not already be an entry with the given key.  We create our own
 * copy of `str`. */
void
csp_string_map_insert_sized(struct csp_string_map *map, csp_id id,
                            const char *str, size_t str_length);

struct csp_string_map_iterator {
    struct csp_map_iterator iter;
};

void
csp_string_map_get_iterator(const struct csp_string_map *map,
                            struct csp_string_map_iterator *iter);

bool
csp_string_map_iterator_done(struct csp_string_map_iterator *iter);

void
csp_string_map_iterator_advance(struct csp_string_map_iterator *iter);

csp_id
csp_string_map_iterator_get_key(const struct csp_string_map_iterator *iter);

const char *
csp_string_map_iterator_get_value(const struct csp_string_map_iterator *iter);

#define csp_string_map_foreach(map, iter)            \
    for (csp_string_map_get_iterator((map), (iter)); \
         !csp_string_map_iterator_done((iter));      \
         csp_string_map_iterator_advance((iter)))

#endif /* HST_STRING_MAP_H */
