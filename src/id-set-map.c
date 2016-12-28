/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-set-map.h"

#include <assert.h>
#include <stdlib.h>

#include "ccan/likely/likely.h"
#include "id-set.h"
#include "map.h"

void
csp_id_set_map_init(struct csp_id_set_map *map)
{
    csp_map_init(&map->map);
}

static void
csp_id_set_map_free_entry(void *ud, void *entry)
{
    struct csp_id_set *set = entry;
    csp_id_set_done(set);
    free(set);
}

void
csp_id_set_map_done(struct csp_id_set_map *map)
{
    csp_map_done(&map->map, csp_id_set_map_free_entry, NULL);
}

static bool
csp_id_set_map_entry_eq(void *ud, const void *entry1, const void *entry2)
{
    const struct csp_id_set *set1 = entry1;
    const struct csp_id_set *set2 = entry2;
    return csp_id_set_eq(set1, set2);
}

bool
csp_id_set_map_eq(const struct csp_id_set_map *map1,
                  const struct csp_id_set_map *map2)
{
    return csp_map_eq(&map1->map, &map2->map, csp_id_set_map_entry_eq, NULL);
}

const struct csp_id_set *
csp_id_set_map_get(const struct csp_id_set_map *map, csp_id id)
{
    return csp_map_get(&map->map, id);
}

static void
csp_id_set_map_init_entry(void *ud, void **ventry)
{
    struct csp_id_set *set = malloc(sizeof(struct csp_id_set));
    assert(set != NULL);
    csp_id_set_init(set);
    *ventry = set;
}

bool
csp_id_set_map_insert(struct csp_id_set_map *map, csp_id from, csp_id to)
{
    struct csp_id_set *set =
            csp_map_insert(&map->map, from, csp_id_set_map_init_entry, NULL);
    return csp_id_set_add(set, to);
}

bool
csp_id_set_map_remove(struct csp_id_set_map *map, csp_id from, csp_id to)
{
    struct csp_id_set *set = csp_map_get(&map->map, from);
    if (unlikely(set == NULL)) {
        return false;
    } else {
        return csp_id_set_remove(set, to);
    }
}

void
csp_id_set_map_get_iterator(const struct csp_id_set_map *map,
                            struct csp_id_set_map_iterator *iter)
{
    csp_map_get_iterator(&map->map, &iter->iter);
}

bool
csp_id_set_map_iterator_done(struct csp_id_set_map_iterator *iter)
{
    return csp_map_iterator_done(&iter->iter);
}

void
csp_id_set_map_iterator_advance(struct csp_id_set_map_iterator *iter)
{
    csp_map_iterator_advance(&iter->iter);
}

csp_id
csp_id_set_map_iterator_get_from(const struct csp_id_set_map_iterator *iter)
{
    return iter->iter.key;
}

const struct csp_id_set *
csp_id_set_map_iterator_get_tos(const struct csp_id_set_map_iterator *iter)
{
    return *iter->iter.value;
}
