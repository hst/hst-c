/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "refinement.h"

#include <string.h>

#include "basics.h"
#include "behavior.h"
#include "environment.h"
#include "event.h"
#include "id-set.h"
#include "normalization.h"
#include "test-case-harness.h"
#include "test-cases.h"

/* The test cases in this file verify that we've implemented refinement
 * correctly. */

/*------------------------------------------------------------------------------
 * Closures
 */

/* Verify the closure of the given CSP₀ process.  `event` should be event to
 * calculate the closure for. */
static void
check_closure_(const char *filename, unsigned int line, struct csp *csp,
               struct csp_process_factory process_,
               struct csp_event_factory event_,
               struct csp_process_set_factory expected_)
{
    struct csp_process *process;
    const struct csp_event *event;
    const struct csp_process_set *expected;
    struct csp_process_set actual;
    csp_process_set_init(&actual);
    process = csp_process_factory_create(csp, process_);
    event = csp_event_factory_create(csp, event_);
    expected = csp_process_set_factory_create(csp, expected_);
    csp_process_set_add(&actual, process);
    csp_find_process_closure(csp, event, &actual);
    check_(filename, line, csp_process_set_eq(&actual, expected));
    csp_process_set_done(&actual);
}

#define check_closure ADD_FILE_AND_LINE(check_closure_)

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
 * Bisimulation
 */

/* Prenormalizes and bisimulates `root_process`.  Then verify that all of the
 * prenormalized nodes in `equivalent_nodes` belong to the same equivalence
 * class. */
static void
check_bisimulation(struct csp_process_factory root_,
                   struct csp_process_set_factory_array *equivalent_)
{
    size_t i;
    struct csp *csp;
    struct csp_process *root;
    struct csp_process *prenormalized;
    struct csp_equivalences equiv;
    csp_id class_id = CSP_ID_NONE;
    const struct csp_process_set *actual;
    struct csp_process_set expected;
    /* Initialize everything. */
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    csp_process_set_init(&expected);
    /* Load the main process. */
    root = csp_process_factory_create(csp, root_);
    /* Prenormalize and bisimulate the root process. */
    prenormalized = csp_prenormalize_process(csp, root);
    csp_calculate_bisimulation(csp, prenormalized, &equiv);
    /* Construct a set containing the normalized nodes that are expected to be
     * equivalent. */
    check(equivalent_->count > 0);
    for (i = 0; i < equivalent_->count; i++) {
        const struct csp_process_set *node_processes =
                csp_process_set_factory_create(csp, equivalent_->sets[i]);
        struct csp_process *node =
                csp_prenormalized_process_new(csp, node_processes);
        csp_process_set_add(&expected, node);
        if (i == 0) {
            /* While building up this set grab the ID of the equivalence class
             * that the first normalized node belongs to. */
            class_id = csp_equivalences_get_class(&equiv, node);
            check_id_ne(class_id, CSP_ID_NONE);
        }
    }
    /* Find all of the nodes that are in the same equivalence class as the first
     * expected node. */
    actual = csp_equivalences_get_members(&equiv, class_id);
    /* And verify that the actual and expected sets of equivalent nodes are
     * equal. */
    check_process_set_eq(csp, actual, &expected);
    /* Clean up. */
    csp_equivalences_done(&equiv);
    csp_process_set_done(&expected);
    csp_free(csp);
}

TEST_CASE_GROUP("bisimulation");

TEST_CASE("a→a→STOP ~ a→a→STOP (separate branches)") {
    const char *process =
            "let "
            "  root = □ {b→A,c→D} "
            "  A = □ {a→B} "
            "  B = □ {a→C} "
            "  C = □ {} "
            "  D = □ {a→E} "
            "  E = □ {a→F} "
            "  F = □ {} "
            "within root";
    check_bisimulation(csp0(process), process_sets(csp0s("A@0"), csp0s("D@0")));
    check_bisimulation(csp0(process), process_sets(csp0s("B@0"), csp0s("E@0")));
    check_bisimulation(csp0(process), process_sets(csp0s("C@0"), csp0s("F@0")));
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
    check_bisimulation(csp0(process), process_sets(csp0s("A@0")));
    check_bisimulation(csp0(process), process_sets(csp0s("B@0", "D@0")));
    check_bisimulation(csp0(process), process_sets(csp0s("C@0", "E@0")));
}

/*------------------------------------------------------------------------------
 * Normalization
 */

static void
check_normalize(struct csp_process_factory process_,
                struct csp_process_set_factory expected_closure_)
{
    struct csp *csp;
    struct csp_process *process;
    struct csp_process *prenormalized;
    struct csp_process *normalized;
    struct csp_process_set actual_closure;
    const struct csp_process_set *expected_closure;
    check_alloc(csp, csp_new());
    process = csp_process_factory_create(csp, process_);
    prenormalized = csp_prenormalize_process(csp, process);
    normalized = csp_normalize_process(csp, prenormalized);
    csp_process_set_init(&actual_closure);
    csp_normalized_process_get_processes(csp, normalized, &actual_closure);
    expected_closure = csp_process_set_factory_create(csp, expected_closure_);
    check(csp_process_set_eq(&actual_closure, expected_closure));
    csp_process_set_done(&actual_closure);
    csp_free(csp);
}

static void
check_normalized_edge(struct csp_process_factory root_,
                      struct csp_process_set_factory from_,
                      struct csp_event_factory event_,
                      struct csp_process_set_factory to_)
{
    struct csp *csp;
    struct csp_process *root;
    struct csp_process *prenormalized;
    struct csp_process *normalized;
    const struct csp_process_set *from;
    struct csp_process *from_prenormalized;
    struct csp_process *from_normalized;
    const struct csp_process_set *to;
    struct csp_process *to_prenormalized;
    struct csp_process *to_normalized;
    struct csp_process *actual;
    const struct csp_event *event;
    check_alloc(csp, csp_new());
    root = csp_process_factory_create(csp, root_);
    prenormalized = csp_prenormalize_process(csp, root);
    normalized = csp_normalize_process(csp, prenormalized);
    from = csp_process_set_factory_create(csp, from_);
    from_prenormalized = csp_prenormalized_process_new(csp, from);
    from_normalized =
            csp_normalized_subprocess(csp, normalized, from_prenormalized);
    to = csp_process_set_factory_create(csp, to_);
    to_prenormalized = csp_prenormalized_process_new(csp, to);
    to_normalized =
            csp_normalized_subprocess(csp, normalized, to_prenormalized);
    event = csp_event_factory_create(csp, event_);
    actual = csp_process_get_single_after(csp, from_normalized, event);
    check(actual == to_normalized);
    csp_free(csp);
}

TEST_CASE_GROUP("normalization");

TEST_CASE("a→a→STOP ~ a→a→STOP (separate branches)") {
    const char *process =
            "let "
            "  root = □ {b→A,c→D} "
            "  A = □ {a→B} "
            "  B = □ {a→C} "
            "  C = □ {} "
            "  D = □ {a→E} "
            "  E = □ {a→F} "
            "  F = □ {} "
            "within root";
    /* Normalize the process and verify we get all of the nodes and edges we
     * expect. */
    check_normalize(csp0(process), csp0s("root@0"));
    check_normalized_edge(csp0(process), csp0s("A@0", "D@0"), event("a"),
                          csp0s("B@0", "E@0"));
    check_normalized_edge(csp0(process), csp0s("B@0", "E@0"), event("a"),
                          csp0s("C@0", "F@0"));
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
    /* Normalize the process and verify we get all of the nodes and edges we
     * expect. */
    check_normalize(csp0(process), csp0s("A@0"));
    check_normalized_edge(csp0(process), csp0s("A@0"), event("a"),
                          csp0s("B@0", "D@0"));
    check_normalized_edge(csp0(process), csp0s("B@0", "D@0"), event("a"),
                          csp0s("C@0", "E@0"));
}

/*------------------------------------------------------------------------------
 * Traces refinement
 */

static void
check_traces_refinement(struct csp_process_factory spec_,
                        struct csp_process_factory impl_)
{
    struct csp *csp;
    struct csp_process *spec;
    struct csp_process *impl;
    check_alloc(csp, csp_new());
    spec = csp_process_factory_create(csp, spec_);
    impl = csp_process_factory_create(csp, impl_);
    check(csp_check_traces_refinement(csp, spec, impl));
    csp_free(csp);
}

static void
xcheck_traces_refinement(struct csp_process_factory spec_,
                         struct csp_process_factory impl_)
{
    struct csp *csp;
    struct csp_process *spec;
    struct csp_process *impl;
    check_alloc(csp, csp_new());
    spec = csp_process_factory_create(csp, spec_);
    impl = csp_process_factory_create(csp, impl_);
    check(!csp_check_traces_refinement(csp, spec, impl));
    csp_free(csp);
}

TEST_CASE_GROUP("traces refinement");

TEST_CASE("STOP ⊑T STOP")
{
    check_traces_refinement(csp0("STOP"), csp0("STOP"));
}

TEST_CASE("STOP ⋤T a → STOP")
{
    xcheck_traces_refinement(csp0("STOP"), csp0("a → STOP"));
}

TEST_CASE("STOP ⋤T a → STOP □ b → STOP")
{
    xcheck_traces_refinement(csp0("STOP"), csp0("a → STOP □ b → STOP"));
}

TEST_CASE("STOP ⋤T a → STOP ⊓ b → STOP")
{
    xcheck_traces_refinement(csp0("STOP"), csp0("a → STOP ⊓ b → STOP"));
}

TEST_CASE("a → STOP ⊑T STOP")
{
    check_traces_refinement(csp0("a → STOP"), csp0("STOP"));
}

TEST_CASE("a → STOP ⊑T a → STOP")
{
    check_traces_refinement(csp0("a → STOP"), csp0("a → STOP"));
}

TEST_CASE("a → STOP ⋤T a → STOP □ b → STOP")
{
    xcheck_traces_refinement(csp0("a → STOP"), csp0("a → STOP □ b → STOP"));
}

TEST_CASE("a → STOP ⋤T a → STOP ⊓ b → STOP")
{
    xcheck_traces_refinement(csp0("a → STOP"), csp0("a → STOP ⊓ b → STOP"));
}

TEST_CASE("a → STOP □ b → STOP ⊑T STOP")
{
    check_traces_refinement(csp0("a → STOP □ b → STOP"), csp0("STOP"));
}

TEST_CASE("a → STOP □ b → STOP ⊑T a → STOP")
{
    check_traces_refinement(csp0("a → STOP □ b → STOP"), csp0("a → STOP"));
}

TEST_CASE("a → STOP □ b → STOP ⊑T a → STOP □ b → STOP")
{
    check_traces_refinement(csp0("a → STOP □ b → STOP"),
                            csp0("a → STOP □ b → STOP"));
}

TEST_CASE("a → STOP □ b → STOP ⊑T a → STOP ⊓ b → STOP")
{
    check_traces_refinement(csp0("a → STOP □ b → STOP"),
                            csp0("a → STOP ⊓ b → STOP"));
}

TEST_CASE("a → STOP ⊓ b → STOP ⊑T STOP")
{
    check_traces_refinement(csp0("a → STOP ⊓ b → STOP"), csp0("STOP"));
}

TEST_CASE("a → STOP ⊓ b → STOP ⊑T a → STOP")
{
    check_traces_refinement(csp0("a → STOP ⊓ b → STOP"), csp0("a → STOP"));
}

TEST_CASE("a → STOP ⊓ b → STOP ⊑T a → STOP □ b → STOP")
{
    check_traces_refinement(csp0("a → STOP ⊓ b → STOP"),
                            csp0("a → STOP □ b → STOP"));
}

TEST_CASE("a → STOP ⊓ b → STOP ⊑T a → STOP ⊓ b → STOP")
{
    check_traces_refinement(csp0("a → STOP ⊓ b → STOP"),
                            csp0("a → STOP ⊓ b → STOP"));
}
