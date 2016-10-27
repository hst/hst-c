/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "hst.h"
#include "test-cases.h"
#include "test-case-harness.h"

/* The test cases in this file verify that we've implemented refinement
 * correctly. */

/*------------------------------------------------------------------------------
 * Normalized LTS
 */

#define add_normalized_node(lts, id, processes) \
    do { \
        struct csp_id_set  __set; \
        csp_id_set_init(&__set); \
        fill_csp0_set(&__set, processes); \
        check(csp_normalized_lts_add_node((lts), &__set, &(id))); \
        csp_id_set_done(&__set); \
    } while (0)

#define check_normalized_node(lts, id, processes) \
    do { \
        struct csp_id_set  __expected; \
        const struct csp_id_set  *__actual; \
        csp_id_set_init(&__expected); \
        fill_csp0_set(&__expected, processes); \
        __actual = csp_normalized_lts_get_node_processes((lts), (id)); \
        check(csp_id_set_eq(__actual, &__expected)); \
        csp_id_set_done(&__expected); \
    } while (0)

TEST_CASE_GROUP("normalized LTSes");

TEST_CASE("can build normalized LTS") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    csp_id  id1;
    csp_id  id2;
    csp_id  id3;
    csp_id  id4;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Add some nodes to the normalized LTS. */
    add_normalized_node(lts, id1, ("STOP"));
    add_normalized_node(lts, id2, ("a → STOP", "b → c → STOP"));
    add_normalized_node(lts, id3, ("a → STOP"));
    add_normalized_node(lts, id4, ("a → STOP □ b → STOP"));
    /* Verify that the nodes map to the right process sets. */
    check_normalized_node(lts, id1, ("STOP"));
    check_normalized_node(lts, id2, ("a → STOP", "b → c → STOP"));
    check_normalized_node(lts, id3, ("a → STOP"));
    check_normalized_node(lts, id4, ("a → STOP □ b → STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

#define add_duplicate_normalized_node(lts, id, processes) \
    do { \
        csp_id  __new_id; \
        struct csp_id_set  __set; \
        csp_id_set_init(&__set); \
        fill_csp0_set(&__set, processes); \
        check(!csp_normalized_lts_add_node((lts), &__set, &__new_id)); \
        check_id_eq(__new_id, (id)); \
        csp_id_set_done(&__set); \
    } while (0)

TEST_CASE("can detect duplicate normalized LTS nodes") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    csp_id  id;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Add some nodes to the normalized LTS.  Ensure that we are notified when a
     * duplicate node is added. */
    add_normalized_node(lts, id, ("STOP"));
    add_duplicate_normalized_node(lts, id, ("STOP"));
    /* For sets of processes, the order shouldn't matter. */
    add_normalized_node(lts, id, ("a → STOP", "b → c → STOP"));
    add_duplicate_normalized_node(lts, id, ("a → STOP", "b → c → STOP"));
    add_duplicate_normalized_node(lts, id, ("b → c → STOP", "a → STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

#define check_normalized_edge(lts, from, event, expected) \
    do { \
        csp_id  __actual = \
            csp_normalized_lts_get_edge((lts), (from), (event)); \
        check_id_eq(__actual, (expected)); \
    } while (0)

TEST_CASE("can add edges to normalized LTS") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    csp_id  id1;
    csp_id  id2;
    csp_id  id3;
    csp_id  id4;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Add some nodes to the normalized LTS. */
    add_normalized_node(lts, id1, ("STOP"));
    add_normalized_node(lts, id2, ("a → STOP", "b → c → STOP"));
    add_normalized_node(lts, id3, ("a → STOP"));
    add_normalized_node(lts, id4, ("a → STOP □ b → STOP"));
    /* Then add some edges. */
    csp_normalized_lts_add_edge(lts, id1, 1, id2);
    csp_normalized_lts_add_edge(lts, id1, 2, id2);
    csp_normalized_lts_add_edge(lts, id1, 3, id3);
    csp_normalized_lts_add_edge(lts, id2, 1, id4);
    /* Verify that the edges exist. */
    check_normalized_edge(lts, id1, 1, id2);
    check_normalized_edge(lts, id1, 2, id2);
    check_normalized_edge(lts, id1, 3, id3);
    check_normalized_edge(lts, id2, 1, id4);
    check_normalized_edge(lts, id2, 2, CSP_NODE_NONE);
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

/*------------------------------------------------------------------------------
 * Closures
 */

/* Verify the closure of the given CSP₀ process.  `event` should be event to
 * calculate the closure for.  `expected` should be a (possibly empty)
 * parenthesized list of CSP₀ processes. */
#define check_closure(csp0, event, expected) \
    do { \
        csp_id  __process; \
        csp_id  __event; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
        csp_id_set_init(&__actual); \
        csp_id_set_init(&__expected); \
        check0(csp_load_csp0_string(csp, (csp0), &__process)); \
        csp_id_set_fill_single(&__actual, __process); \
        __event = csp_get_event_id(csp, (event)); \
        csp_process_find_closure(csp, __event, &__actual); \
        fill_csp0_set(&__expected, expected); \
        check_with_msg( \
                csp_id_set_eq(&__actual, &__expected), \
                "closure(" event ", " csp0 ") == {" \
                CPPMAGIC_JOIN(",", CPPMAGIC_UNPACK(expected)) \
                "}"); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
    } while (0)

TEST_CASE_GROUP("closures");

TEST_CASE("a → a → a → STOP □ a → b → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure("a → a → a → STOP □ a → b → STOP", "τ",
                  ("a → a → a → STOP □ a → b → STOP"));
    check_closure("a → a → a → STOP □ a → b → STOP", "a",
                  ("a → a → a → STOP □ a → b → STOP",
                   "a → a → STOP", "a → STOP", "STOP", "b → STOP"));
    check_closure("a → a → a → STOP □ a → b → STOP", "b",
                  ("a → a → a → STOP □ a → b → STOP"));
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → STOP □ b → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure("a → STOP □ b → STOP", "τ", ("a → STOP □ b → STOP"));
    check_closure("a → STOP □ b → STOP", "a", ("a → STOP □ b → STOP", "STOP"));
    check_closure("a → STOP □ b → STOP", "b", ("a → STOP □ b → STOP", "STOP"));
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → STOP ⊓ (b → STOP ⊓ c → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure("a → STOP ⊓ (b → STOP ⊓ c → STOP)", "τ",
                  ("a → STOP ⊓ (b → STOP ⊓ c → STOP)", "b → STOP ⊓ c → STOP",
                   "a → STOP", "b → STOP", "c → STOP"));
    check_closure("a → STOP ⊓ (b → STOP ⊓ c → STOP)", "a",
                  ("a → STOP ⊓ (b → STOP ⊓ c → STOP)"));
    check_closure("a → STOP ⊓ (b → STOP ⊓ c → STOP)", "b",
                  ("a → STOP ⊓ (b → STOP ⊓ c → STOP)"));
    /* Clean up. */
    csp_free(csp);
}
