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

static void
add_normalized_node(struct csp *csp, struct csp_normalized_lts *lts, csp_id *id,
                    struct csp_id_set_factory processes)
{
    const struct csp_id_set  *process_ids =
        csp_id_set_factory_create(csp, processes);
    check(csp_normalized_lts_add_node(lts, process_ids, id));
}

static void
check_normalized_node(struct csp *csp, struct csp_normalized_lts *lts, csp_id id,
                      struct csp_id_set_factory processes)
{
    const struct csp_id_set  *actual =
        csp_normalized_lts_get_node_processes(lts, id);
    const struct csp_id_set  *process_ids =
        csp_id_set_factory_create(csp, processes);
    check_set_eq(actual, process_ids);
}

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
    add_normalized_node(csp, lts, &id1, csp0s("STOP"));
    add_normalized_node(csp, lts, &id2, csp0s("a → STOP", "b → c → STOP"));
    add_normalized_node(csp, lts, &id3, csp0s("a → STOP"));
    add_normalized_node(csp, lts, &id4, csp0s("a → STOP □ b → STOP"));
    /* Verify that the nodes map to the right process sets. */
    check_normalized_node(csp, lts, id1, csp0s("STOP"));
    check_normalized_node(csp, lts, id2, csp0s("a → STOP", "b → c → STOP"));
    check_normalized_node(csp, lts, id3, csp0s("a → STOP"));
    check_normalized_node(csp, lts, id4, csp0s("a → STOP □ b → STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

static void
add_duplicate_normalized_node(struct csp *csp, struct csp_normalized_lts *lts,
                              csp_id id, struct csp_id_set_factory processes)
{
    const struct csp_id_set  *process_ids =
        csp_id_set_factory_create(csp, processes);
    csp_id  new_id;
    check(!csp_normalized_lts_add_node(lts, process_ids, &new_id));
    check_id_eq(new_id, id);
}

TEST_CASE("can detect duplicate normalized LTS nodes") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    csp_id  id;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Add some nodes to the normalized LTS.  Ensure that we are notified when a
     * duplicate node is added. */
    add_normalized_node(csp, lts, &id, csp0s("STOP"));
    add_duplicate_normalized_node(csp, lts, id, csp0s("STOP"));
    /* For sets of processes, the order shouldn't matter. */
    add_normalized_node(csp, lts, &id, csp0s("a → STOP", "b → c → STOP"));
    add_duplicate_normalized_node(
            csp, lts, id, csp0s("a → STOP", "b → c → STOP"));
    add_duplicate_normalized_node(
            csp, lts, id, csp0s("b → c → STOP", "a → STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

static void
check_normalized_edge_by_id(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event, csp_id expected)
{
    csp_id  actual = csp_normalized_lts_get_edge(lts, from, event);
    check_id_eq(actual, expected);
}

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
    add_normalized_node(csp, lts, &id1, csp0s("STOP"));
    add_normalized_node(csp, lts, &id2, csp0s("a → STOP", "b → c → STOP"));
    add_normalized_node(csp, lts, &id3, csp0s("a → STOP"));
    add_normalized_node(csp, lts, &id4, csp0s("a → STOP □ b → STOP"));
    /* Then add some edges. */
    csp_normalized_lts_add_edge(lts, id1, 1, id2);
    csp_normalized_lts_add_edge(lts, id1, 2, id2);
    csp_normalized_lts_add_edge(lts, id1, 3, id3);
    csp_normalized_lts_add_edge(lts, id2, 1, id4);
    /* Verify that the edges exist. */
    check_normalized_edge_by_id(lts, id1, 1, id2);
    check_normalized_edge_by_id(lts, id1, 2, id2);
    check_normalized_edge_by_id(lts, id1, 3, id3);
    check_normalized_edge_by_id(lts, id2, 1, id4);
    check_normalized_edge_by_id(lts, id2, 2, CSP_NODE_NONE);
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

/*------------------------------------------------------------------------------
 * Closures
 */

/* Verify the closure of the given CSP₀ process.  `event` should be event to
 * calculate the closure for. */
static void
check_closure(struct csp *csp, struct csp_id_factory process,
              struct csp_id_factory event, struct csp_id_set_factory expected)
{
    csp_id  process_id;
    csp_id  event_id;
    const struct csp_id_set  *expected_set;
    struct csp_id_set  actual;
    csp_id_set_init(&actual);
    process_id = csp_id_factory_create(csp, process);
    event_id = csp_id_factory_create(csp, event);
    expected_set = csp_id_set_factory_create(csp, expected);
    csp_id_set_fill_single(&actual, process_id);
    csp_process_find_closure(csp, event_id, &actual);
    check_set_eq(&actual, expected_set);
    csp_id_set_done(&actual);
}

TEST_CASE_GROUP("closures");

TEST_CASE("a → a → a → STOP □ a → b → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure(csp, csp0("a → a → a → STOP □ a → b → STOP"), event("τ"),
                  csp0s("a → a → a → STOP □ a → b → STOP"));
    check_closure(csp, csp0("a → a → a → STOP □ a → b → STOP"), event("a"),
                  csp0s("a → a → a → STOP □ a → b → STOP",
                        "a → a → STOP", "a → STOP", "STOP", "b → STOP"));
    check_closure(csp, csp0("a → a → a → STOP □ a → b → STOP"), event("b"),
                  csp0s("a → a → a → STOP □ a → b → STOP"));
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → STOP □ b → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure(csp, csp0("a → STOP □ b → STOP"), event("τ"),
                  csp0s("a → STOP □ b → STOP"));
    check_closure(csp, csp0("a → STOP □ b → STOP"), event("a"),
                  csp0s("a → STOP □ b → STOP", "STOP"));
    check_closure(csp, csp0("a → STOP □ b → STOP"), event("b"),
                  csp0s("a → STOP □ b → STOP", "STOP"));
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → STOP ⊓ (b → STOP ⊓ c → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure(csp, csp0("a → STOP ⊓ (b → STOP ⊓ c → STOP)"), event("τ"),
                  csp0s("a → STOP ⊓ (b → STOP ⊓ c → STOP)",
                        "b → STOP ⊓ c → STOP",
                        "a → STOP", "b → STOP", "c → STOP"));
    check_closure(csp, csp0("a → STOP ⊓ (b → STOP ⊓ c → STOP)"), event("a"),
                  csp0s("a → STOP ⊓ (b → STOP ⊓ c → STOP)"));
    check_closure(csp, csp0("a → STOP ⊓ (b → STOP ⊓ c → STOP)"), event("b"),
                  csp0s("a → STOP ⊓ (b → STOP ⊓ c → STOP)"));
    /* Clean up. */
    csp_free(csp);
}

/*------------------------------------------------------------------------------
 * Prenormalization
 */

TEST_CASE_GROUP("prenormalization");

static void
check_prenormalize(struct csp *csp, struct csp_normalized_lts *lts,
                   struct csp_id_factory process,
                   struct csp_id_set_factory expected_closure)
{
    csp_id  process_id;
    csp_id  normalized_node;
    const struct csp_id_set  *processes;
    const struct csp_id_set  *expected_closure_set;
    process_id = csp_id_factory_create(csp, process);
    expected_closure_set = csp_id_set_factory_create(csp, expected_closure);
    normalized_node = csp_process_prenormalize(csp, lts, process_id);
    processes = csp_normalized_lts_get_node_processes(lts, normalized_node);
    check_set_eq(processes, expected_closure_set);
}

static void
check_normalized_edge(struct csp *csp, struct csp_normalized_lts *lts,
                      struct csp_id_set_factory from_node,
                      struct csp_id_factory event,
                      struct csp_id_set_factory to_node)
{
    const struct csp_id_set  *from_node_set;
    const struct csp_id_set  *to_node_set;
    csp_id  event_id;
    csp_id  from_node_id;
    csp_id  actual;
    csp_id  expected;
    from_node_set = csp_id_set_factory_create(csp, from_node);
    event_id = csp_id_factory_create(csp, event);
    to_node_set = csp_id_set_factory_create(csp, to_node);
    check(!csp_normalized_lts_add_node(lts, from_node_set, &from_node_id));
    check(!csp_normalized_lts_add_node(lts, to_node_set, &expected));
    actual = csp_normalized_lts_get_edge(lts, from_node_id, event_id);
    check_id_eq(actual, expected);
}

TEST_CASE("a → STOP") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → STOP"), csp0s("a → STOP"));
    check_normalized_edge(
            csp, lts, csp0s("a → STOP"), event("a"), csp0s("STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE("a → STOP □ b → STOP") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → STOP □ b → STOP"),
                       csp0s("a → STOP □ b → STOP"));
    check_normalized_edge(
            csp, lts, csp0s("a → STOP □ b → STOP"), event("a"), csp0s("STOP"));
    check_normalized_edge(
            csp, lts, csp0s("a → STOP □ b → STOP"), event("b"), csp0s("STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE("a → STOP ⊓ b → STOP") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → STOP ⊓ b → STOP"),
                       csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"));
    check_normalized_edge(
            csp, lts, csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"),
            event("a"), csp0s("STOP"));
    check_normalized_edge(
            csp, lts, csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"),
            event("b"), csp0s("STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE("a → SKIP ; b → STOP") {
    struct csp  *csp;
    struct csp_normalized_lts  *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → SKIP ; b → STOP"),
                       csp0s("a → SKIP ; b → STOP"));
    check_normalized_edge(
            csp, lts, csp0s("a → SKIP ; b → STOP"), event("a"),
            csp0s("SKIP ; b → STOP", "b → STOP"));
    check_normalized_edge(
            csp, lts, csp0s("SKIP ; b → STOP", "b → STOP"), event("b"),
            csp0s("STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}
