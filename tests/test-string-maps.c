/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "string-map.h"

#include "test-case-harness.h"

TEST_CASE_GROUP("ID→string maps");

TEST_CASE("can create empty map")
{
    struct csp_string_map_iterator iter;
    struct csp_string_map map;
    csp_string_map_init(&map);
    csp_string_map_get_iterator(&map, &iter);
    check(csp_string_map_iterator_done(&iter));
    csp_string_map_done(&map);
}

TEST_CASE("can add entries")
{
    struct csp_string_map map;
    csp_string_map_init(&map);
    csp_string_map_insert(&map, 10, "a");
    check_streq(csp_string_map_get(&map, 10), "a");
    csp_string_map_done(&map);
}

TEST_CASE("can iterate through map entries")
{
    struct csp_string_map_iterator iter;
    struct csp_string_map map;
    size_t count = 0;
    csp_string_map_init(&map);
    csp_string_map_insert(&map, 10, "a");
    csp_string_map_insert(&map, 30, "c");
    csp_string_map_insert(&map, 20, "b");
    csp_string_map_foreach (&map, &iter) {
        count++;
    }
    check(count == 3);
    csp_string_map_done(&map);
}

TEST_CASE("can get map contents via iterator")
{
    struct csp_string_map_iterator iter;
    struct csp_string_map map;
    csp_string_map_init(&map);
    csp_string_map_insert(&map, 10, "a");
    csp_string_map_insert(&map, 30, "c");
    csp_string_map_insert(&map, 20, "b");
    csp_string_map_get_iterator(&map, &iter);
    check(!csp_string_map_iterator_done(&iter));
    check_id_eq(csp_string_map_iterator_get_key(&iter), 10);
    check_streq(csp_string_map_iterator_get_value(&iter), "a");
    csp_string_map_iterator_advance(&iter);
    check(!csp_string_map_iterator_done(&iter));
    check_id_eq(csp_string_map_iterator_get_key(&iter), 20);
    check_streq(csp_string_map_iterator_get_value(&iter), "b");
    csp_string_map_iterator_advance(&iter);
    check(!csp_string_map_iterator_done(&iter));
    check_id_eq(csp_string_map_iterator_get_key(&iter), 30);
    check_streq(csp_string_map_iterator_get_value(&iter), "c");
    csp_string_map_iterator_advance(&iter);
    check(csp_string_map_iterator_done(&iter));
    csp_string_map_done(&map);
}
