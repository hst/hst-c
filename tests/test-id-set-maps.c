/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-set-map.h"

#include "id-set.h"
#include "test-case-harness.h"

TEST_CASE_GROUP("ID→id_set maps");

TEST_CASE("can create empty map")
{
    struct csp_id_set_map_iterator iter;
    struct csp_id_set_map map;
    csp_id_set_map_init(&map);
    csp_id_set_map_get_iterator(&map, &iter);
    check(csp_id_set_map_iterator_done(&iter));
    csp_id_set_map_done(&map);
}

TEST_CASE("can add entries")
{
    struct csp_id_set_map map;
    csp_id_set_map_init(&map);
    csp_id_set_map_insert(&map, 10, 20);
    csp_id_set_map_insert(&map, 20, 30);
    csp_id_set_map_insert(&map, 20, 40);
    check_set_eq(csp_id_set_map_get(&map, 10), id_set(20));
    check_set_eq(csp_id_set_map_get(&map, 20), id_set(30, 40));
    csp_id_set_map_done(&map);
}

TEST_CASE("can iterate through map entries")
{
    struct csp_id_set_map_iterator iter;
    struct csp_id_set_map map;
    size_t count = 0;
    csp_id_set_map_init(&map);
    csp_id_set_map_insert(&map, 20, 30);
    csp_id_set_map_insert(&map, 10, 20);
    csp_id_set_map_insert(&map, 20, 40);
    csp_id_set_map_foreach (&map, &iter) {
        count++;
    }
    check(count == 2);
    csp_id_set_map_done(&map);
}

TEST_CASE("can get map contents via iterator")
{
    struct csp_id_set_map_iterator iter;
    struct csp_id_set_map map;
    csp_id_set_map_init(&map);
    csp_id_set_map_insert(&map, 20, 30);
    csp_id_set_map_insert(&map, 10, 20);
    csp_id_set_map_insert(&map, 20, 40);
    csp_id_set_map_get_iterator(&map, &iter);
    check(!csp_id_set_map_iterator_done(&iter));
    check_id_eq(csp_id_set_map_iterator_get_from(&iter), 10);
    check_set_eq(csp_id_set_map_iterator_get_tos(&iter), id_set(20));
    csp_id_set_map_iterator_advance(&iter);
    check(!csp_id_set_map_iterator_done(&iter));
    check_id_eq(csp_id_set_map_iterator_get_from(&iter), 20);
    check_set_eq(csp_id_set_map_iterator_get_tos(&iter), id_set(30, 40));
    csp_id_set_map_iterator_advance(&iter);
    check(csp_id_set_map_iterator_done(&iter));
    csp_id_set_map_done(&map);
}
