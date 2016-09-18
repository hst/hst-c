/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "hst.h"
#include "test-case-harness.h"

TEST_CASE_GROUP("environments");

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

#define check_streq(actual, expected) \
    check_with_msg(strcmp((actual), (expected)) == 0, \
            "Expected \"%s\", got \"%s\"", (expected), (actual))

#define check_id_eq(id1, id2) \
    check_with_msg((id1) == (id2), \
            "Expected IDs to be equal, got 0x%08lx and 0x%08lx", \
            (id1), (id2))

#define check_id_ne(id1, id2) \
    check_with_msg((id1) != (id2), \
            "Expected IDs to be unequal, got 0x%08lx", (id1))

#define build_set(set, ...) \
    do { \
        csp_id  __to_add[] = { __VA_ARGS__ }; \
        size_t  __count = sizeof(__to_add) / sizeof(__to_add[0]); \
        struct csp_id_set_builder  builder; \
        csp_id_set_builder_init(&builder); \
        csp_id_set_builder_add_many(&builder, __count, __to_add); \
        csp_id_set_build((set), &builder); \
        csp_id_set_builder_done(&builder); \
    } while (0)

TEST_CASE("predefined events exist") {
    static const char* const TAU = "τ";
    static const char* const TICK = "✔";
    struct csp  *csp;
    check_alloc(csp, csp_new());
    check_streq(csp_get_event_name(csp, csp->tau), TAU);
    check_streq(csp_get_event_name(csp, csp->tick), TICK);
    csp_free(csp);
}

TEST_CASE("can create events") {
    struct csp  *csp;
    check_alloc(csp, csp_new());
    check_with_msg(csp_get_event_name(csp, 0) == NULL,
                   "Should get a name for undefined event");
    check_streq(csp_get_event_name(csp, csp_get_event_id(csp, "a")), "a");
    check_streq(csp_get_event_name(csp, csp_get_event_id(csp, "b")), "b");
    csp_free(csp);
}

TEST_CASE("predefined STOP process exists") {
    struct csp  *csp;
    struct csp_id_set  set;
    /* Create the CSP environment. */
    csp_id_set_init(&set);
    check_alloc(csp, csp_new());
    /* Verify the initials set of the STOP process. */
    csp_process_get_initials(csp, csp->stop, &set);
    check_size(set, 0);
    /* Verify the afters of τ. */
    csp_process_get_afters(csp, csp->stop, csp->tau, &set);
    check_size(set, 0);
    /* Verify the afters of ✔. */
    csp_process_get_afters(csp, csp->stop, csp->tick, &set);
    check_size(set, 0);
    /* Clean up. */
    csp_id_set_done(&set);
    csp_free(csp);
}

TEST_CASE("predefined SKIP process exists") {
    struct csp  *csp;
    struct csp_id_set  set;
    /* Create the CSP environment. */
    csp_id_set_init(&set);
    check_alloc(csp, csp_new());
    /* Verify the initials set of the SKIP process. */
    csp_process_get_initials(csp, csp->skip, &set);
    check_size(set, 1);
    check_elements(set, csp->tick);
    /* Verify the afters of τ. */
    csp_process_get_afters(csp, csp->skip, csp->tau, &set);
    check_size(set, 0);
    /* Verify the afters of ✔. */
    csp_process_get_afters(csp, csp->skip, csp->tick, &set);
    check_size(set, 1);
    check_elements(set, csp->stop);
    /* Clean up. */
    csp_id_set_done(&set);
    csp_free(csp);
}

TEST_CASE("base process IDs should be reproducible") {
    static struct csp_id_scope  scope;
    check_id_eq(csp_id_start(&scope), csp_id_start(&scope));
}

TEST_CASE("distinct scopes should produce different IDs") {
    static struct csp_id_scope  scope1;
    static struct csp_id_scope  scope2;
    check_id_ne(csp_id_start(&scope1), csp_id_start(&scope2));
}

TEST_CASE("ID-derived process IDs should be reproducible") {
    static struct csp_id_scope  scope;
    csp_id  base = csp_id_start(&scope);
    check_id_eq(csp_id_add_id(base, 0), csp_id_add_id(base, 0));
    check_id_eq(csp_id_add_id(base, 10), csp_id_add_id(base, 10));
    check_id_eq(csp_id_add_id(base, 100), csp_id_add_id(base, 100));
}

TEST_CASE("distinct IDs should produce different IDs") {
    static struct csp_id_scope  scope;
    csp_id  base = csp_id_start(&scope);
    check_id_ne(csp_id_add_id(base, 0), csp_id_add_id(base, 10));
    check_id_ne(csp_id_add_id(base, 0), csp_id_add_id(base, 100));
    check_id_ne(csp_id_add_id(base, 10), csp_id_add_id(base, 100));
}

TEST_CASE("set-derived process IDs should be reproducible") {
    static struct csp_id_scope  scope;
    struct csp_id_set  set1;
    struct csp_id_set  set2;
    csp_id  base = csp_id_start(&scope);
    csp_id_set_init(&set1);
    csp_id_set_init(&set2);
    build_set(&set1, 1, 2, 3, 4);
    build_set(&set2, 1, 2, 3, 4);
    check_id_eq(csp_id_add_id_set(base, &set1), csp_id_add_id_set(base, &set1));
    check_id_eq(csp_id_add_id_set(base, &set1), csp_id_add_id_set(base, &set2));
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("distinct sets should produce different IDs") {
    static struct csp_id_scope  scope;
    struct csp_id_set  set1;
    struct csp_id_set  set2;
    csp_id  base = csp_id_start(&scope);
    csp_id_set_init(&set1);
    csp_id_set_init(&set2);
    build_set(&set1, 1, 2, 3, 4);
    build_set(&set2, 5, 6, 7, 8);
    check_id_ne(csp_id_add_id_set(base, &set1), csp_id_add_id_set(base, &set2));
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("name-derived process IDs should be reproducible") {
    static struct csp_id_scope  scope;
    csp_id  base = csp_id_start(&scope);
    check_id_eq(csp_id_add_name(base, "a"), csp_id_add_name(base, "a"));
    check_id_eq(csp_id_add_name(base, "b"), csp_id_add_name(base, "b"));
    check_id_eq(csp_id_add_name(base, "c"), csp_id_add_name(base, "c"));
}

TEST_CASE("distinct names should produce different IDs") {
    static struct csp_id_scope  scope;
    csp_id  base = csp_id_start(&scope);
    check_id_ne(csp_id_add_name(base, "a"), csp_id_add_name(base, "b"));
    check_id_ne(csp_id_add_name(base, "a"), csp_id_add_name(base, "c"));
    check_id_ne(csp_id_add_name(base, "b"), csp_id_add_name(base, "c"));
}
