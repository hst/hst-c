/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"
#include "test-case-harness.h"

#define CSP_ID_SET_FIRST_ALLOCATION_COUNT  32

TEST_CASE_GROUP("identifier sets");

#define check_size(set, expected) \
    check_with_msg((set).count == (expected), \
            "Expected set to have size %zu, got %zu", \
            (size_t) (expected), (set).count)

#define check_elements(set, ...) \
    do { \
        csp_id  __expected[] = { __VA_ARGS__ }; \
        size_t  __count = sizeof(__expected) / sizeof(__expected[0]); \
        size_t  __i; \
        for (__i = 0; __i < __count; __i++) { \
            check_with_msg((set).ids[__i] == __expected[__i], \
                    "Expected set[%zu] to be %lu, got %lu", \
                    __i, __expected[__i], (set).ids[__i]); \
        } \
    } while (0)

#define check_range(set, count) \
    do { \
        check_size(set, count); \
        size_t  __i; \
        for (__i = 0; __i < (count); __i++) { \
            check_with_msg((set).ids[__i] == __i, \
                    "Expected set[%zu] to be %zu, got %lu", \
                    __i, __i, (set).ids[__i]); \
        } \
    } while (0)

static void
csp_id_set_builder_add_range(struct csp_id_set_builder *builder, size_t count)
{
    size_t  i;
    for (i = 0; i < count; i++) {
        csp_id_set_builder_add(builder, i);
    }
}

TEST_CASE("can create empty set") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_done(&set);
    check_size(set, 0);
    csp_id_set_builder_done(&builder);
}

TEST_CASE("can add individual ids") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add(&builder, 0);
    csp_id_set_builder_add(&builder, 5);
    csp_id_set_builder_add(&builder, 1);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    check_size(set, 3);
    check_elements(set, 0, 1, 5);
    csp_id_set_done(&set);
}

TEST_CASE("can add duplicate individual ids") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add(&builder, 0);
    csp_id_set_builder_add(&builder, 5);
    csp_id_set_builder_add(&builder, 1);
    csp_id_set_builder_add(&builder, 5);
    csp_id_set_builder_add(&builder, 0);
    csp_id_set_builder_add(&builder, 0);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    check_size(set, 3);
    check_elements(set, 0, 1, 5);
    csp_id_set_done(&set);
}

TEST_CASE("can add bulk ids") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id  to_add[] = {0, 5, 1};
    size_t  to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add_many(&builder, to_add_count, to_add);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    check_size(set, 3);
    check_elements(set, 0, 1, 5);
    csp_id_set_done(&set);
}

TEST_CASE("can add duplicate bulk ids") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id  to_add[] = {0, 5, 1, 5, 0, 0};
    size_t  to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add_many(&builder, to_add_count, to_add);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    check_size(set, 3);
    check_elements(set, 0, 1, 5);
    csp_id_set_done(&set);
}

TEST_CASE("can merge sets") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set1;
    struct csp_id_set  set2;
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add(&builder, 0);
    csp_id_set_builder_add(&builder, 1);
    csp_id_set_init(&set1);
    csp_id_set_build(&set1, &builder);
    csp_id_set_builder_add(&builder, 5);
    csp_id_set_builder_merge(&builder, &set1);
    csp_id_set_init(&set2);
    csp_id_set_build(&set2, &builder);
    csp_id_set_builder_done(&builder);
    check_size(set2, 3);
    check_elements(set2, 0, 1, 5);
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("unlocking a set builder empties it") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add(&builder, 0);
    csp_id_set_builder_add(&builder, 5);
    csp_id_set_builder_add(&builder, 1);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    check_size(set, 0);
    csp_id_set_done(&set);
}

TEST_CASE("can clone a small set") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set1;
    struct csp_id_set  set2;
    csp_id  to_add[] = {0, 5, 1};
    size_t  to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    /* Create a set. */
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add_many(&builder, to_add_count, to_add);
    csp_id_set_init(&set1);
    csp_id_set_build(&set1, &builder);
    csp_id_set_builder_done(&builder);
    /* Then create a copy of it. */
    csp_id_set_init(&set2);
    csp_id_set_clone(&set2, &set1);
    /* And verify its contents. */
    check_size(set2, 3);
    check_elements(set2, 0, 1, 5);
    /* Clean up. */
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("can clone a large set") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set1;
    struct csp_id_set  set2;
    /* Create a set. */
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_add_range(
            &builder, CSP_ID_SET_FIRST_ALLOCATION_COUNT + 1);
    csp_id_set_init(&set1);
    csp_id_set_build(&set1, &builder);
    csp_id_set_builder_done(&builder);
    /* Then create a copy of it. */
    csp_id_set_init(&set2);
    csp_id_set_clone(&set2, &set1);
    /* And verify its contents. */
    check_range(set2, CSP_ID_SET_FIRST_ALLOCATION_COUNT + 1);
    /* Clean up. */
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("can spill over into allocated storage") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    /* Fill the set with too many elements to fit into the preallocated internal
     * storage, but few enough to fit into the default-sized heap-allocated
     * buffer . */
    csp_id_set_builder_add_range(&builder, CSP_ID_SET_INTERNAL_SIZE + 1);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    /* Verify that we got a valid set. */
    check_range(set, CSP_ID_SET_INTERNAL_SIZE + 1);
    csp_id_set_done(&set);
}

TEST_CASE("can spill over into large allocated storage") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    /* Fill the set with too many elements to fit into the preallocated internal
     * storage, and too many too fit into the default-sized heap-allocated
     * buffer. */
    csp_id_set_builder_add_range(
            &builder, CSP_ID_SET_FIRST_ALLOCATION_COUNT + 1);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    /* Verify that we got a valid set. */
    check_range(set, CSP_ID_SET_FIRST_ALLOCATION_COUNT + 1);
    csp_id_set_done(&set);
}

TEST_CASE("can reallocate allocated storage") {
    struct csp_id_set_builder  builder;
    struct csp_id_set  set;
    csp_id_set_builder_init(&builder);
    /* Fill the set with one too many elements to fit into the preallocated
     * internal storage, causing an initial allocation. */
    csp_id_set_builder_add_range(&builder, CSP_ID_SET_INTERNAL_SIZE + 1);
    csp_id_set_init(&set);
    csp_id_set_build(&set, &builder);
    /* Then fill the set some more, to cause us to reallocate the heap-allocated
     * storage. */
    csp_id_set_builder_add_range(
            &builder, CSP_ID_SET_FIRST_ALLOCATION_COUNT + 1);
    csp_id_set_build(&set, &builder);
    csp_id_set_builder_done(&builder);
    /* Verify that we got a valid set. */
    check_range(set, CSP_ID_SET_FIRST_ALLOCATION_COUNT + 1);
    csp_id_set_done(&set);
}
