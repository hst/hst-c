/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "hst.h"
#include "test-case-harness.h"
#include "test-cases.h"

/* The test cases in this file verify that we've implemented refinement
 * correctly. */

/*------------------------------------------------------------------------------
 * Normalized LTS
 */

static void
add_normalized_node(struct csp *csp, struct csp_normalized_lts *lts, csp_id *id,
                    struct csp_id_set_factory processes)
{
    const struct csp_id_set *process_ids =
            csp_id_set_factory_create(csp, processes);
    check(csp_normalized_lts_add_node(lts, process_ids, id));
}

static void
check_normalized_node(struct csp *csp, struct csp_normalized_lts *lts,
                      csp_id id, struct csp_id_set_factory processes)
{
    const struct csp_id_set *actual =
            csp_normalized_lts_get_node_processes(lts, id);
    const struct csp_id_set *process_ids =
            csp_id_set_factory_create(csp, processes);
    check_set_eq(actual, process_ids);
}

static void
check_normalized_node_set(struct csp *csp, struct csp_normalized_lts *lts,
                          struct csp_id_set_factory ids)
{
    struct csp_id_set_builder builder;
    struct csp_id_set actual;
    const struct csp_id_set *id_set;
    csp_id_set_builder_init(&builder);
    csp_id_set_init(&actual);
    id_set = csp_id_set_factory_create(csp, ids);
    csp_normalized_lts_build_all_nodes(lts, &builder);
    csp_id_set_build(&actual, &builder);
    check_set_eq(&actual, id_set);
    csp_id_set_builder_done(&builder);
    csp_id_set_done(&actual);
}

static void
check_normalized_node_traces_behavior(struct csp *csp,
                                      struct csp_normalized_lts *lts,
                                      struct csp_id_set_factory processes,
                                      struct csp_id_set_factory events)
{
    csp_id id;
    const struct csp_id_set *process_set;
    const struct csp_id_set *event_set;
    const struct csp_behavior *behavior;
    process_set = csp_id_set_factory_create(csp, processes);
    event_set = csp_id_set_factory_create(csp, events);
    check(!csp_normalized_lts_add_node(lts, process_set, &id));
    behavior = csp_normalized_lts_get_node_behavior(lts, id);
    check_set_eq(&behavior->initials, event_set);
}

TEST_CASE_GROUP("normalized LTSes");

TEST_CASE("can build normalized LTS")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    csp_id id1;
    csp_id id2;
    csp_id id3;
    csp_id id4;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    /* Add some nodes to the normalized LTS. */
    add_normalized_node(csp, lts, &id1, csp0s("STOP"));
    add_normalized_node(csp, lts, &id2, csp0s("a → STOP", "b → c → STOP"));
    add_normalized_node(csp, lts, &id3, csp0s("a → STOP"));
    add_normalized_node(csp, lts, &id4, csp0s("a → STOP □ b → STOP"));
    /* Verify that all of the nodes exist. */
    check_normalized_node_set(csp, lts, ids(id1, id2, id3, id4));
    /* Verify that the nodes map to the right process sets. */
    check_normalized_node(csp, lts, id1, csp0s("STOP"));
    check_normalized_node(csp, lts, id2, csp0s("a → STOP", "b → c → STOP"));
    check_normalized_node(csp, lts, id3, csp0s("a → STOP"));
    check_normalized_node(csp, lts, id4, csp0s("a → STOP □ b → STOP"));
    /* Verify that the nodes have the expected merged behavior. */
    check_normalized_node_traces_behavior(csp, lts, csp0s("STOP"), events());
    check_normalized_node_traces_behavior(
            csp, lts, csp0s("a → STOP", "b → c → STOP"), events("a", "b"));
    check_normalized_node_traces_behavior(csp, lts, csp0s("a → STOP"),
                                          events("a"));
    check_normalized_node_traces_behavior(
            csp, lts, csp0s("a → STOP □ b → STOP"), events("a", "b"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

static void
add_duplicate_normalized_node(struct csp *csp, struct csp_normalized_lts *lts,
                              csp_id id, struct csp_id_set_factory processes)
{
    const struct csp_id_set *process_ids =
            csp_id_set_factory_create(csp, processes);
    csp_id new_id;
    check(!csp_normalized_lts_add_node(lts, process_ids, &new_id));
    check_id_eq(new_id, id);
}

TEST_CASE("can detect duplicate normalized LTS nodes")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    csp_id id;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    /* Add some nodes to the normalized LTS.  Ensure that we are notified when a
     * duplicate node is added. */
    add_normalized_node(csp, lts, &id, csp0s("STOP"));
    add_duplicate_normalized_node(csp, lts, id, csp0s("STOP"));
    /* For sets of processes, the order shouldn't matter. */
    add_normalized_node(csp, lts, &id, csp0s("a → STOP", "b → c → STOP"));
    add_duplicate_normalized_node(csp, lts, id,
                                  csp0s("a → STOP", "b → c → STOP"));
    add_duplicate_normalized_node(csp, lts, id,
                                  csp0s("b → c → STOP", "a → STOP"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

static void
check_normalized_edge_by_id(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event, csp_id expected)
{
    csp_id actual = csp_normalized_lts_get_edge(lts, from, event);
    check_id_eq(actual, expected);
}

static void
check_normalized_edges(struct csp *csp, struct csp_normalized_lts *lts,
                       csp_id from, struct csp_id_pair_array_factory edges)
{
    struct csp_id_pair_array actual;
    struct csp_id_pair_array *edges_array;
    csp_id_pair_array_init(&actual);
    edges_array = csp_id_pair_array_factory_create(csp, edges);
    csp_normalized_lts_get_node_edges(lts, from, &actual);
    check_pair_array_eq(&actual, edges_array);
    csp_id_pair_array_done(&actual);
}

TEST_CASE("can add edges to normalized LTS")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    csp_id id1;
    csp_id id2;
    csp_id id3;
    csp_id id4;
    /* Create the CSP environment and LTS. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
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
    check_normalized_edges(csp, lts, id1, pairs(id(1), id(id2), id(2), id(id2),
                                                id(3), id(id3)));
    check_normalized_edges(csp, lts, id2, pairs(id(1), id(id4)));
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
    csp_id process_id;
    csp_id event_id;
    const struct csp_id_set *expected_set;
    struct csp_id_set actual;
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

TEST_CASE("a → a → a → STOP □ a → b → STOP")
{
    struct csp *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure(csp, csp0("a → a → a → STOP □ a → b → STOP"), event("τ"),
                  csp0s("a → a → a → STOP □ a → b → STOP"));
    check_closure(csp, csp0("a → a → a → STOP □ a → b → STOP"), event("a"),
                  csp0s("a → a → a → STOP □ a → b → STOP", "a → a → STOP",
                        "a → STOP", "STOP", "b → STOP"));
    check_closure(csp, csp0("a → a → a → STOP □ a → b → STOP"), event("b"),
                  csp0s("a → a → a → STOP □ a → b → STOP"));
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → STOP □ b → STOP")
{
    struct csp *csp;
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

TEST_CASE("a → STOP ⊓ (b → STOP ⊓ c → STOP)")
{
    struct csp *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP □ b → STOP */
    check_closure(
            csp, csp0("a → STOP ⊓ (b → STOP ⊓ c → STOP)"), event("τ"),
            csp0s("a → STOP ⊓ (b → STOP ⊓ c → STOP)", "b → STOP ⊓ c → STOP",
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
    csp_id process_id;
    csp_id normalized_node;
    const struct csp_id_set *processes;
    const struct csp_id_set *expected_closure_set;
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
    const struct csp_id_set *from_node_set;
    const struct csp_id_set *to_node_set;
    csp_id event_id;
    csp_id from_node_id;
    csp_id actual;
    csp_id expected;
    from_node_set = csp_id_set_factory_create(csp, from_node);
    event_id = csp_id_factory_create(csp, event);
    to_node_set = csp_id_set_factory_create(csp, to_node);
    check(!csp_normalized_lts_add_node(lts, from_node_set, &from_node_id));
    check(!csp_normalized_lts_add_node(lts, to_node_set, &expected));
    actual = csp_normalized_lts_get_edge(lts, from_node_id, event_id);
    check_id_eq(actual, expected);
}

TEST_CASE("a → STOP")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → STOP"), csp0s("a → STOP"));
    check_normalized_edge(csp, lts, csp0s("a → STOP"), event("a"),
                          csp0s("STOP"));
    /* And verify that the normalized nodes have the behavior we expect. */
    check_normalized_node_traces_behavior(csp, lts, csp0s("a → STOP"),
                                          events("a"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE("a → STOP □ b → STOP")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → STOP □ b → STOP"),
                       csp0s("a → STOP □ b → STOP"));
    check_normalized_edge(csp, lts, csp0s("a → STOP □ b → STOP"), event("a"),
                          csp0s("STOP"));
    check_normalized_edge(csp, lts, csp0s("a → STOP □ b → STOP"), event("b"),
                          csp0s("STOP"));
    /* And verify that the normalized nodes have the behavior we expect. */
    check_normalized_node_traces_behavior(
            csp, lts, csp0s("a → STOP □ b → STOP"), events("a", "b"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE("a → STOP ⊓ b → STOP")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → STOP ⊓ b → STOP"),
                       csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"));
    check_normalized_edge(csp, lts,
                          csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"),
                          event("a"), csp0s("STOP"));
    check_normalized_edge(csp, lts,
                          csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"),
                          event("b"), csp0s("STOP"));
    /* And verify that the normalized nodes have the behavior we expect. */
    check_normalized_node_traces_behavior(
            csp, lts, csp0s("a → STOP ⊓ b → STOP", "a → STOP", "b → STOP"),
            events("a", "b"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE("a → SKIP ; b → STOP")
{
    struct csp *csp;
    struct csp_normalized_lts *lts;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    /* Prenormalize the process and verify we get all of the edges we expect. */
    check_prenormalize(csp, lts, csp0("a → SKIP ; b → STOP"),
                       csp0s("a → SKIP ; b → STOP"));
    check_normalized_edge(csp, lts, csp0s("a → SKIP ; b → STOP"), event("a"),
                          csp0s("SKIP ; b → STOP", "b → STOP"));
    check_normalized_edge(csp, lts, csp0s("SKIP ; b → STOP", "b → STOP"),
                          event("b"), csp0s("STOP"));
    /* And verify that the normalized nodes have the behavior we expect. */
    check_normalized_node_traces_behavior(
            csp, lts, csp0s("a → SKIP ; b → STOP"), events("a"));
    check_normalized_node_traces_behavior(
            csp, lts, csp0s("SKIP ; b → STOP", "b → STOP"), events("b"));
    /* Clean up. */
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

/*------------------------------------------------------------------------------
 * Bisimulation
 */

/* Load `root_process`, prenormalize all of `prenormalized_processes`, and then
 * bisimulate the result.  Then verify that all of the normalized nodes in
 * `equivalent_nodes` belong to the same equivalence class. */
static void
check_bisimulation(struct csp_id_factory root_process,
                   struct csp_id_set_factory prenormalized_processes,
                   struct normalized_node_array *equivalent_nodes)
{
    size_t i;
    struct csp *csp;
    struct csp_normalized_lts *lts;
    const struct csp_id_set *prenormalized;
    struct csp_equivalences equiv;
    const struct csp_id_set *node_processes;
    csp_id node_id;
    csp_id class_id = CSP_ID_NONE;
    struct csp_id_set_builder builder;
    struct csp_id_set actual;
    struct csp_id_set expected;
    /* Initialize everything. */
    check_alloc(csp, csp_new());
    check_alloc(lts, csp_normalized_lts_new(csp, CSP_TRACES));
    csp_equivalences_init(&equiv);
    csp_id_set_builder_init(&builder);
    csp_id_set_init(&actual);
    csp_id_set_init(&expected);
    /* Load the main process. */
    csp_id_factory_create(csp, root_process);
    /* Prenormalize all of the requested processes. */
    prenormalized = csp_id_set_factory_create(csp, prenormalized_processes);
    for (i = 0; i < prenormalized->count; i++) {
        csp_process_prenormalize(csp, lts, prenormalized->ids[i]);
    }
    /* Bisimulate the prenormalized LTS. */
    csp_normalized_lts_bisimulate(lts, &equiv);
    /* Construct a set containing the IDs of the normalized nodes that are
     * expected to be equivalent. */
    check(equivalent_nodes->count > 0);
    for (i = 0; i < equivalent_nodes->count; i++) {
        node_processes =
                csp_id_set_factory_create(csp, equivalent_nodes->nodes[i]);
        csp_normalized_lts_add_node(lts, node_processes, &node_id);
        csp_id_set_builder_add(&builder, node_id);
        if (i == 0) {
            /* While building up this set grab the ID of the equivalence class
             * that the first normalized node belongs to. */
            class_id = csp_equivalences_get_class(&equiv, node_id);
            check_id_ne(class_id, CSP_ID_NONE);
        }
    }
    csp_id_set_build(&expected, &builder);
    /* Find all of the nodes that are in the same equivalence class as the first
     * expected node. */
    csp_equivalences_build_members(&equiv, class_id, &builder);
    csp_id_set_build(&actual, &builder);
    /* And verify that the actual and expected sets of equivalent nodes are
     * equal. */
    check_set_eq(&actual, &expected);
    /* Clean up. */
    csp_equivalences_done(&equiv);
    csp_id_set_builder_done(&builder);
    csp_id_set_done(&actual);
    csp_id_set_done(&expected);
    csp_normalized_lts_free(lts);
    csp_free(csp);
}

TEST_CASE_GROUP("bisimulation");

TEST_CASE("a→a→STOP ~ a→a→STOP (separate branches)") {
    const char *process =
            "let "
            "  A = □ {a→B} "
            "  B = □ {a→C} "
            "  C = □ {} "
            "  D = □ {a→E} "
            "  E = □ {a→F} "
            "  F = □ {} "
            "within A";
    check_bisimulation(csp0(process), csp0s("A@0", "D@0"),
                       normalized_nodes(csp0s("A@0"), csp0s("D@0")));
    check_bisimulation(csp0(process), csp0s("A@0", "D@0"),
                       normalized_nodes(csp0s("B@0"), csp0s("E@0")));
    check_bisimulation(csp0(process), csp0s("A@0", "D@0"),
                       normalized_nodes(csp0s("C@0"), csp0s("F@0")));
}

TEST_CASE("a→a→STOP ~ a→a→STOP (single head)") {
    const char *process =
            "let "
            "  A = □ {a→B, a→D} "
            "  B = □ {a→C} "
            "  C = □ {} "
            "  D = □ {a→E} "
            "  E = □ {} "
            "within A";
    check_bisimulation(csp0(process), csp0s("A@0"),
                       normalized_nodes(csp0s("A@0")));
    check_bisimulation(csp0(process), csp0s("A@0"),
                       normalized_nodes(csp0s("B@0", "D@0")));
    check_bisimulation(csp0(process), csp0s("A@0"),
                       normalized_nodes(csp0s("C@0", "E@0")));
}
