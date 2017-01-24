/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "map.h"

#include <assert.h>
#include <stdbool.h>

#include "ccan/avl/avl.h"
#include "ccan/compiler/compiler.h"

static int
csp_map_cmp(const void *key1, const void *key2)
{
    if (key1 < key2) {
        return -1;
    } else if (key1 > key2) {
        return 1;
    } else {
        return 0;
    }
}

void
csp_map_init(struct csp_map *map)
{
    map->avl = avl_new(csp_map_cmp);
    assert(map->avl != NULL);
}

void
csp_map_done(struct csp_map *map, csp_map_free_entry_f *free_entry, void *ud)
{
    if (free_entry != NULL) {
        struct csp_map_iterator iter;
        csp_map_foreach (map, &iter) {
            void *value = csp_map_iterator_get_value(&iter);
            free_entry(ud, value);
        }
    }
    avl_free(map->avl);
}

bool
csp_map_eq(const struct csp_map *map1, const struct csp_map *map2,
           csp_map_entry_eq_f *entry_eq, void *ud)
{
    struct csp_map_iterator iter;
    csp_map_foreach (map1, &iter) {
        void *value1 = csp_map_iterator_get_value(&iter);
        void *value2 = csp_map_get(map2, csp_map_iterator_get_key(&iter));
        if (value2 == NULL) {
            return false;
        }
        if (!entry_eq(ud, value1, value2)) {
            return false;
        }
    }
    return true;
}

bool
csp_map_empty(const struct csp_map *map)
{
    return avl_count(map->avl) == 0;
}

size_t
csp_map_size(const struct csp_map *map)
{
    return avl_count(map->avl);
}

void *
csp_map_get(const struct csp_map *map, csp_id id)
{
    return avl_lookup(map->avl, (void *) id);
}

bool
csp_map_insert(struct csp_map *map, csp_id id, void *value)
{
    return avl_insert(map->avl, (void *) id, value);
}

void
csp_map_remove(struct csp_map *map, csp_id id, csp_map_free_entry_f *free_entry,
               void *ud)
{
    if (free_entry != NULL) {
        void *entry = csp_map_get(map, id);
        if (entry != NULL) {
            free_entry(ud, entry);
        }
    }
    avl_remove(map->avl, (void *) id);
}

void
csp_map_get_iterator(const struct csp_map *map, struct csp_map_iterator *iter)
{
    avl_iter_begin(&iter->iter, map->avl, FORWARD);
}

csp_id
csp_map_iterator_get_key(const struct csp_map_iterator *iter)
{
    return (uintptr_t) iter->iter.key;
}

void *
csp_map_iterator_get_value(const struct csp_map_iterator *iter)
{
    return (void *) iter->iter.value;
}

bool
csp_map_iterator_done(struct csp_map_iterator *iter)
{
    return iter->iter.node == NULL;
}

void
csp_map_iterator_advance(struct csp_map_iterator *iter)
{
    avl_iter_next(&iter->iter);
}
