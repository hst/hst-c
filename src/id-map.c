/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-map.h"

#include "basics.h"
#include "map.h"

void
csp_id_map_init(struct csp_id_map *map)
{
    csp_map_init(&map->map);
}

void
csp_id_map_done(struct csp_id_map *map)
{
    csp_map_done(&map->map, NULL, NULL);
}

static bool
csp_id_map_entry_eq(void *ud, const void *entry1, const void *entry2)
{
    csp_id id1 = (uintptr_t) entry1;
    csp_id id2 = (uintptr_t) entry2;
    return id1 == id2;
}

bool
csp_id_map_eq(const struct csp_id_map *map1, const struct csp_id_map *map2)
{
    return csp_map_eq(&map1->map, &map2->map, csp_id_map_entry_eq, NULL);
}

csp_id
csp_id_map_get(const struct csp_id_map *map, csp_id id)
{
    void *entry = csp_map_get(&map->map, id);
    if (entry == NULL) {
        return CSP_ID_NONE;
    } else {
        return (uintptr_t) entry;
    }
}

csp_id
csp_id_map_insert(struct csp_id_map *map, csp_id from, csp_id to)
{
    void **entry = csp_map_at(&map->map, from);
    csp_id result = (uintptr_t) *entry;
    *entry = (void *) to;
    return result;
}

void
csp_id_map_get_iterator(const struct csp_id_map *map,
                        struct csp_id_map_iterator *iter)
{
    csp_map_get_iterator(&map->map, &iter->iter);
}

bool
csp_id_map_iterator_done(struct csp_id_map_iterator *iter)
{
    return csp_map_iterator_done(&iter->iter);
}

void
csp_id_map_iterator_advance(struct csp_id_map_iterator *iter)
{
    csp_map_iterator_advance(&iter->iter);
}

csp_id
csp_id_map_iterator_get_from(const struct csp_id_map_iterator *iter)
{
    return iter->iter.key;
}

csp_id
csp_id_map_iterator_get_to(const struct csp_id_map_iterator *iter)
{
    return (uintptr_t) *iter->iter.value;
}
