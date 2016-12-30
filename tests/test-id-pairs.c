/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-pair.h"

#include "test-case-harness.h"

#define CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT 32

TEST_CASE_GROUP("ID pair arrays");

TEST_CASE("can create empty array via factory")
{
    struct csp_id_pair_array *array =
            csp_id_pair_array_factory_create(NULL, pairs());
    check(array->count == 0);
}

TEST_CASE("can create 1-element array via factory")
{
    struct csp_id_pair_array *array =
            csp_id_pair_array_factory_create(NULL, pairs(id(10), id(20)));
    check(array->count == 1);
    check_id_eq(array->pairs[0].from, 10);
    check_id_eq(array->pairs[0].to, 20);
}

TEST_CASE("can create 5-element array via factory")
{
    struct csp_id_pair_array *array = csp_id_pair_array_factory_create(
            NULL, pairs(id(10), id(15), id(20), id(25), id(30), id(35), id(40),
                        id(45), id(50), id(55)));
    check(array->count == 5);
    check_id_eq(array->pairs[0].from, 10);
    check_id_eq(array->pairs[0].to, 15);
    check_id_eq(array->pairs[1].from, 20);
    check_id_eq(array->pairs[1].to, 25);
    check_id_eq(array->pairs[2].from, 30);
    check_id_eq(array->pairs[2].to, 35);
    check_id_eq(array->pairs[3].from, 40);
    check_id_eq(array->pairs[3].to, 45);
    check_id_eq(array->pairs[4].from, 50);
    check_id_eq(array->pairs[4].to, 55);
}

TEST_CASE("can spill over into allocated storage")
{
    struct csp_id_pair_array array;
    csp_id_pair_array_init(&array);
    /* Resize the array with too many elements to fit into the preallocated
     * internal storage, but few enough to fit into the default-sized
     * heap-allocated buffer. */
    csp_id_pair_array_ensure_size(&array, CSP_ID_PAIR_ARRAY_INTERNAL_SIZE + 1);
    /* Verify that we're not using the internal storage anymore. */
    check(array.pairs != array.internal);
    check(array.allocated_count == CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT);
    csp_id_pair_array_done(&array);
}

TEST_CASE("can spill over into large allocated storage")
{
    struct csp_id_pair_array array;
    csp_id_pair_array_init(&array);
    /* Resize the array with too many elements to fit into the preallocated
     * internal storage, and too many too fit into the default-sized
     * heap-allocated buffer. */
    csp_id_pair_array_ensure_size(&array,
                                  CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT + 1);
    /* Verify that we're not using the internal storage anymore. */
    check(array.pairs != array.internal);
    check(array.allocated_count ==
          CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT * 2);
    csp_id_pair_array_done(&array);
}

TEST_CASE("can reallocate allocated storage")
{
    struct csp_id_pair_array array;
    csp_id_pair_array_init(&array);
    /* Resize the array with one too many elements to fit into the preallocated
     * internal storage, causing an initial allocation. */
    csp_id_pair_array_ensure_size(&array, CSP_ID_PAIR_ARRAY_INTERNAL_SIZE + 1);
    /* Then resize the array again, to cause us to reallocate the heap-allocated
     * storage. */
    csp_id_pair_array_ensure_size(&array,
                                  CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT + 1);
    /* Verify that we're not using the internal storage anymore. */
    check(array.pairs != array.internal);
    check(array.allocated_count ==
          CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT * 2);
    csp_id_pair_array_done(&array);
}

TEST_CASE_GROUP("ID pair sets");

#define make_pair_set(...) \
    csp_id_pair_set_factory_create(NULL, pair_set(__VA_ARGS__))

TEST_CASE("can create empty sets via factory")
{
    struct csp_id_pair_set *set = make_pair_set();
    check(set->count == 0);
}

TEST_CASE("can create 1-element set via factory")
{
    struct csp_id_pair_set *set = make_pair_set(id(10), id(20));
    check(set->count == 1);
    check(csp_id_pair_set_contains(set, pair(10, 20)));
}

TEST_CASE("can create 5-element set via factory")
{
    struct csp_id_pair_set *set =
            make_pair_set(id(10), id(15), id(10), id(25), id(30), id(35),
                          id(40), id(45), id(50), id(55));
    check(set->count == 5);
    check(csp_id_pair_set_contains(set, pair(10, 15)));
    check(csp_id_pair_set_contains(set, pair(10, 25)));
    check(csp_id_pair_set_contains(set, pair(30, 35)));
    check(csp_id_pair_set_contains(set, pair(40, 45)));
    check(csp_id_pair_set_contains(set, pair(50, 55)));
}

TEST_CASE("duplicates are ignored when creating set via factory")
{
    struct csp_id_pair_set *set =
            make_pair_set(id(10), id(15), id(10), id(15), id(30), id(35),
                          id(40), id(45), id(50), id(55));
    check(set->count == 4);
    check(csp_id_pair_set_contains(set, pair(10, 15)));
    check(csp_id_pair_set_contains(set, pair(30, 35)));
    check(csp_id_pair_set_contains(set, pair(40, 45)));
    check(csp_id_pair_set_contains(set, pair(50, 55)));
}

TEST_CASE("pairs in set are sorted")
{
    struct csp_id_pair_set *set =
            make_pair_set(id(20), id(45), id(20), id(15), id(20), id(55),
                          id(30), id(35), id(10), id(15));
    check(set->count == 5);
    check_id_eq(set->pairs[0].from, 10);
    check_id_eq(set->pairs[0].to, 15);
    check_id_eq(set->pairs[1].from, 20);
    check_id_eq(set->pairs[1].to, 15);
    check_id_eq(set->pairs[2].from, 20);
    check_id_eq(set->pairs[2].to, 45);
    check_id_eq(set->pairs[3].from, 20);
    check_id_eq(set->pairs[3].to, 55);
    check_id_eq(set->pairs[4].from, 30);
    check_id_eq(set->pairs[4].to, 35);
}

static void
check_union(struct csp_id_pair_set_factory lhs,
            struct csp_id_pair_set_factory rhs,
            struct csp_id_pair_set_factory expected)
{
    struct csp_id_pair_set *lhs_set = csp_id_pair_set_factory_create(NULL, lhs);
    struct csp_id_pair_set *rhs_set = csp_id_pair_set_factory_create(NULL, rhs);
    struct csp_id_pair_set *expected_set =
            csp_id_pair_set_factory_create(NULL, expected);
    csp_id_pair_set_union(lhs_set, rhs_set);
    check(csp_id_pair_set_eq(lhs_set, expected_set));
}

TEST_CASE("can union two sets")
{
    check_union(pair_set(id(10), id(15)), pair_set(id(20), id(25)),
                pair_set(id(10), id(15), id(20), id(25)));
}

TEST_CASE("duplicates are ignored when unioning sets")
{
    check_union(pair_set(id(10), id(15), id(20), id(25)),
                pair_set(id(20), id(25), id(30), id(35)),
                pair_set(id(10), id(15), id(20), id(25), id(30), id(35)));
}

TEST_CASE("can union longer left set")
{
    check_union(pair_set(id(10), id(15), id(20), id(25)),
                pair_set(id(30), id(35)),
                pair_set(id(10), id(15), id(20), id(25), id(30), id(35)));
}

TEST_CASE("can union longer right set")
{
    check_union(pair_set(id(10), id(15)),
                pair_set(id(20), id(25), id(30), id(35)),
                pair_set(id(10), id(15), id(20), id(25), id(30), id(35)));
}
