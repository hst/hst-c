/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "string-map.h"

#include <string.h>

#include "map.h"

void
csp_string_map_init(struct csp_string_map *map)
{
    csp_map_init(&map->map);
}

static void
csp_string_map_free_entry(void *ud, void *entry)
{
    char *str = entry;
    free(str);
}

void
csp_string_map_done(struct csp_string_map *map)
{
    csp_map_done(&map->map, csp_string_map_free_entry, NULL);
}

static bool
csp_string_map_entry_eq(void *ud, const void *entry1, const void *entry2)
{
    const char *str1 = entry1;
    const char *str2 = entry2;
    return strcmp(str1, str2) == 0;
}

bool
csp_string_map_eq(const struct csp_string_map *map1,
                  const struct csp_string_map *map2)
{
    return csp_map_eq(&map1->map, &map2->map, csp_string_map_entry_eq,
                         NULL);
}

const char *
csp_string_map_get(const struct csp_string_map *map, csp_id id)
{
    return csp_map_get(&map->map, id);
}

static void
csp_string_map_init_entry(void *ud, void **ventry)
{
    const char *str = ud;
    char **entry = (char **) ventry;
    *entry = strdup(str);
}

void
csp_string_map_insert(struct csp_string_map *map, csp_id id, const char *str)
{
    csp_map_insert(&map->map, id, csp_string_map_init_entry, (void *) str);
}

struct csp_string_map_sized_string {
    const char *str;
    size_t length;
};

static void
csp_string_map_init_entry_sized(void *ud, void **ventry)
{
    struct csp_string_map_sized_string *sized = ud;
    char **entry = (char **) ventry;
    char *str_copy = malloc(sized->length + 1);
    memcpy(str_copy, sized->str, sized->length);
    str_copy[sized->length] = '\0';
    *entry = str_copy;
}

void
csp_string_map_insert_sized(struct csp_string_map *map, csp_id id,
                            const char *str, size_t str_length)
{
    struct csp_string_map_sized_string sized = {str, str_length};
    csp_map_insert(&map->map, id, csp_string_map_init_entry_sized, &sized);
}

void
csp_string_map_get_iterator(const struct csp_string_map *map,
                            struct csp_string_map_iterator *iter)
{
    csp_map_get_iterator(&map->map, &iter->iter);
}

bool
csp_string_map_iterator_done(struct csp_string_map_iterator *iter)
{
    return csp_map_iterator_done(&iter->iter);
}

void
csp_string_map_iterator_advance(struct csp_string_map_iterator *iter)
{
    csp_map_iterator_advance(&iter->iter);
}

csp_id
csp_string_map_iterator_get_key(const struct csp_string_map_iterator *iter)
{
    return iter->iter.key;
}

const char *
csp_string_map_iterator_get_value(const struct csp_string_map_iterator *iter)
{
    return *iter->iter.value;
}
