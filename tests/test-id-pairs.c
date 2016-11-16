/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"
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
