/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-map.h"

#include "test-case-harness.h"

TEST_CASE_GROUP("ID→ID maps");

TEST_CASE("can create empty map via factory")
{
    struct csp_id_map_iterator iter;
    struct csp_id_map *map = csp_id_map_factory_create(NULL, id_map());
    csp_id_map_get_iterator(map, &iter);
    check(csp_id_map_iterator_done(&iter));
}

TEST_CASE("can create 1-element map via factory")
{
    struct csp_id_map_iterator iter;
    struct csp_id_map *map =
            csp_id_map_factory_create(NULL, id_map(id(10), id(20)));
    csp_id_map_get_iterator(map, &iter);
    check(!csp_id_map_iterator_done(&iter));
    check_id_eq(csp_id_map_iterator_get_from(&iter), 10);
    check_id_eq(csp_id_map_iterator_get_to(&iter), 20);
    csp_id_map_iterator_advance(&iter);
    check(csp_id_map_iterator_done(&iter));
}

TEST_CASE("can create 5-element map via factory")
{
    struct csp_id_map_iterator iter;
    struct csp_id_map *map = csp_id_map_factory_create(
            NULL, id_map(id(10), id(15), id(20), id(25), id(30), id(35), id(40),
                         id(45), id(50), id(55)));
    csp_id_map_get_iterator(map, &iter);
    check(!csp_id_map_iterator_done(&iter));
    check_id_eq(csp_id_map_iterator_get_from(&iter), 10);
    check_id_eq(csp_id_map_iterator_get_to(&iter), 15);
    csp_id_map_iterator_advance(&iter);
    check(!csp_id_map_iterator_done(&iter));
    check_id_eq(csp_id_map_iterator_get_from(&iter), 20);
    check_id_eq(csp_id_map_iterator_get_to(&iter), 25);
    csp_id_map_iterator_advance(&iter);
    check(!csp_id_map_iterator_done(&iter));
    check_id_eq(csp_id_map_iterator_get_from(&iter), 30);
    check_id_eq(csp_id_map_iterator_get_to(&iter), 35);
    csp_id_map_iterator_advance(&iter);
    check(!csp_id_map_iterator_done(&iter));
    check_id_eq(csp_id_map_iterator_get_from(&iter), 40);
    check_id_eq(csp_id_map_iterator_get_to(&iter), 45);
    csp_id_map_iterator_advance(&iter);
    check(!csp_id_map_iterator_done(&iter));
    check_id_eq(csp_id_map_iterator_get_from(&iter), 50);
    check_id_eq(csp_id_map_iterator_get_to(&iter), 55);
    csp_id_map_iterator_advance(&iter);
    check(csp_id_map_iterator_done(&iter));
}

TEST_CASE("can overwrite entries")
{
    struct csp_id_map map;
    const struct csp_id_map *expected;
    csp_id_map_init(&map);
    check_id_eq(csp_id_map_insert(&map, 10, 15), CSP_ID_NONE);
    check_id_eq(csp_id_map_insert(&map, 10, 25), 15);
    expected = csp_id_map_factory_create(NULL, id_map(id(10), id(25)));
    check(csp_id_map_eq(&map, expected));
    csp_id_map_done(&map);
}
