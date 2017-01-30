/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "map.h"

#include <stdbool.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"

void
csp_map_init(struct csp_map *map)
{
    map->entries = NULL;
}

void
csp_map_done(struct csp_map *map, csp_map_free_entry_f *free_entry, void *ud)
{
    UNNEEDED Word_t dummy;
    if (free_entry != NULL) {
        Word_t *ventry;
        csp_id id = 0;
        JLF(ventry, map->entries, id);
        while (ventry != NULL) {
            void *entry = (void *) *ventry;
            free_entry(ud, entry);
            JLN(ventry, map->entries, id);
        }
    }
    JLFA(dummy, map->entries);
}

void
csp_map_get_iterator(const struct csp_map *map, struct csp_map_iterator *iter)
{
    iter->entries = &map->entries;
    iter->key = 0;
    JLF(iter->value, *iter->entries, iter->key);
}

bool
csp_map_iterator_done(struct csp_map_iterator *iter)
{
    return iter->value == NULL;
}

void
csp_map_iterator_advance(struct csp_map_iterator *iter)
{
    JLN(iter->value, *iter->entries, iter->key);
}

bool
csp_map_eq(const struct csp_map *map1, const struct csp_map *map2,
           csp_map_entry_eq_f *entry_eq, void *ud)
{
    struct csp_map_iterator iter;
    csp_map_foreach (map1, &iter) {
        void *entry1 = *iter.value;
        void *entry2 = csp_map_get(map2, iter.key);
        if (entry2 == NULL) {
            return false;
        }
        if (!entry_eq(ud, entry1, entry2)) {
            return false;
        }
    }
    return true;
}

bool
csp_map_empty(const struct csp_map *map)
{
    return map->entries == NULL;
}

size_t
csp_map_size(const struct csp_map *map)
{
    Word_t count;
    JLC(count, map->entries, 0, -1);
    return count;
}

void *
csp_map_get(const struct csp_map *map, csp_id id)
{
    Word_t *ventry;
    JLG(ventry, map->entries, id);
    if (ventry == NULL) {
        return NULL;
    } else {
        void **entry = (void **) ventry;
        return *entry;
    }
}

void **
csp_map_at(struct csp_map *map, csp_id id)
{
    Word_t *ventry;
    JLI(ventry, map->entries, id);
    return (void **) ventry;
}

void *
csp_map_insert(struct csp_map *map, csp_id id, csp_map_init_entry_f *init_entry,
               void *ud)
{
    Word_t *ventry;
    void **entry;
    JLI(ventry, map->entries, id);
    entry = (void **) ventry;
    if (*entry == NULL) {
        init_entry(ud, entry);
    }
    return *entry;
}

void
csp_map_remove(struct csp_map *map, csp_id id, csp_map_free_entry_f *free_entry,
               void *ud)
{
    UNNEEDED int rc;
    if (free_entry != NULL) {
        void *entry = csp_map_get(map, id);
        if (entry != NULL) {
            free_entry(ud, entry);
        }
    }
    JLD(rc, map->entries, id);
}
