/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "operators.h"

#include "basics.h"
#include "behavior.h"
#include "environment.h"
#include "id-set.h"
#include "test-case-harness.h"
#include "test-cases.h"

/* The test cases in this file verify that we've implemented each of the CSP
 * operators correctly: specifically, that they have the right "initials" and
 * "afters" sets, as defined by CSP's operational semantics.
 *
 * We've provided some helper macros that make these test cases easy to write.
 * In particular, you can assume that the CSP₀ parser works as expected; that
 * will have been checked in test-csp0.c.
 */

/* Verify the `initials` of the given CSP₀ process. */
static void
check_process_initials(struct csp_id_factory process,
                       struct csp_id_set_factory expected_initials)
{
    struct csp *csp;
    csp_id process_id;
    struct csp_id_set actual;
    check_alloc(csp, csp_new());
    csp_id_set_init(&actual);
    process_id = csp_id_factory_create(csp, process);
    csp_build_process_initials(csp, process_id, &actual);
    check_set_eq(&actual, csp_id_set_factory_create(csp, expected_initials));
    csp_id_set_done(&actual);
    csp_free(csp);
}

/* Verify the `afters` of the given CSP₀ process after performing `initial`. */
static void
check_process_afters(struct csp_id_factory process,
                     struct csp_id_factory initial,
                     struct csp_id_set_factory expected_afters)
{
    struct csp *csp;
    csp_id process_id;
    csp_id initial_id;
    struct csp_id_set actual;
    check_alloc(csp, csp_new());
    csp_id_set_init(&actual);
    process_id = csp_id_factory_create(csp, process);
    initial_id = csp_id_factory_create(csp, initial);
    csp_build_process_afters(csp, process_id, initial_id, &actual);
    check_set_eq(&actual, csp_id_set_factory_create(csp, expected_afters));
    csp_id_set_done(&actual);
    csp_free(csp);
}

/* Verify the traces behavior of the given CSP₀ process. */
static void
check_process_traces_behavior(struct csp_id_factory process,
                              struct csp_id_set_factory expected_initials)
{
    struct csp *csp;
    csp_id process_id;
    const struct csp_id_set *expected_initials_set;
    struct csp_behavior behavior;
    check_alloc(csp, csp_new());
    csp_behavior_init(&behavior);
    process_id = csp_id_factory_create(csp, process);
    expected_initials_set = csp_id_set_factory_create(csp, expected_initials);
    csp_process_get_behavior(csp, process_id, CSP_TRACES, &behavior);
    check_set_eq(&behavior.initials, expected_initials_set);
    csp_behavior_done(&behavior);
    csp_free(csp);
}

/* Verify the `initials` of a subprocess.  `subprocess` should be a process that
 * has been defined as part of `process`. */
static void
check_process_sub_initials(struct csp_id_factory process,
                           struct csp_id_factory subprocess,
                           struct csp_id_set_factory expected_initials)
{
    struct csp *csp;
    UNNEEDED csp_id process_id;
    csp_id subprocess_id;
    struct csp_id_set actual;
    check_alloc(csp, csp_new());
    csp_id_set_init(&actual);
    process_id = csp_id_factory_create(csp, process);
    subprocess_id = csp_id_factory_create(csp, subprocess);
    csp_build_process_initials(csp, subprocess_id, &actual);
    check_set_eq(&actual, csp_id_set_factory_create(csp, expected_initials));
    csp_id_set_done(&actual);
    csp_free(csp);
}

/* Verify the `afters` of a subprocess after performing `initial`.  `subprocess`
 * should be a process that has been defined as part of `process`. */
static void
check_process_sub_afters(struct csp_id_factory process,
                         struct csp_id_factory subprocess,
                         struct csp_id_factory initial,
                         struct csp_id_set_factory expected_afters)
{
    struct csp *csp;
    UNNEEDED csp_id process_id;
    csp_id subprocess_id;
    csp_id initial_id;
    struct csp_id_set actual;
    check_alloc(csp, csp_new());
    csp_id_set_init(&actual);
    process_id = csp_id_factory_create(csp, process);
    subprocess_id = csp_id_factory_create(csp, subprocess);
    initial_id = csp_id_factory_create(csp, initial);
    csp_build_process_afters(csp, subprocess_id, initial_id, &actual);
    check_set_eq(&actual, csp_id_set_factory_create(csp, expected_afters));
    csp_id_set_done(&actual);
    csp_free(csp);
}

/* Verify the traces behavior of the given CSP₀ process.  `subprocess` should be
 * a process that has been defined as part of `process`. */
static void
check_process_sub_traces_behavior(struct csp_id_factory process,
                                  struct csp_id_factory subprocess,
                                  struct csp_id_set_factory expected_initials)
{
    struct csp *csp;
    UNNEEDED csp_id process_id;
    csp_id subprocess_id;
    const struct csp_id_set *expected_initials_set;
    struct csp_behavior behavior;
    check_alloc(csp, csp_new());
    csp_behavior_init(&behavior);
    process_id = csp_id_factory_create(csp, process);
    subprocess_id = csp_id_factory_create(csp, subprocess);
    expected_initials_set = csp_id_set_factory_create(csp, expected_initials);
    csp_process_get_behavior(csp, subprocess_id, CSP_TRACES, &behavior);
    check_set_eq(&behavior.initials, expected_initials_set);
    csp_behavior_done(&behavior);
    csp_free(csp);
}

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP")
{
    check_process_initials(csp0("STOP □ STOP"), events());
    check_process_afters(csp0("STOP □ STOP"), event("a"), csp0s());
    check_process_traces_behavior(csp0("STOP □ STOP"), events());
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)")
{
    check_process_initials(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"),
                           events("a", "τ"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"), event("a"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"), event("b"),
                         csp0s());
    check_process_afters(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"), event("τ"),
                         csp0s("a → STOP □ b → STOP", "a → STOP □ c → STOP"));
    check_process_traces_behavior(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"),
                                  events("a"));
}

TEST_CASE("(a → STOP) □ (b → STOP)")
{
    check_process_initials(csp0("(a → STOP) □ (b → STOP)"), events("a", "b"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP)"), event("a"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP)"), event("b"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP)"), event("τ"), csp0s());
    check_process_traces_behavior(csp0("(a → STOP) □ (b → STOP)"),
                                  events("a", "b"));
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}")
{
    check_process_initials(csp0("□ {a → STOP, b → STOP, c → STOP}"),
                           events("a", "b", "c"));
    check_process_afters(csp0("□ {a → STOP, b → STOP, c → STOP}"), event("a"),
                         csp0s("STOP"));
    check_process_afters(csp0("□ {a → STOP, b → STOP, c → STOP}"), event("b"),
                         csp0s("STOP"));
    check_process_afters(csp0("□ {a → STOP, b → STOP, c → STOP}"), event("c"),
                         csp0s("STOP"));
    check_process_afters(csp0("□ {a → STOP, b → STOP, c → STOP}"), event("τ"),
                         csp0s());
    check_process_traces_behavior(csp0("□ {a → STOP, b → STOP, c → STOP}"),
                                  events("a", "b", "c"));
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP")
{
    check_process_initials(csp0("STOP ⊓ STOP"), events("τ"));
    check_process_afters(csp0("STOP ⊓ STOP"), event("τ"), csp0s("STOP"));
    check_process_afters(csp0("STOP ⊓ STOP"), event("a"), csp0s());
    check_process_traces_behavior(csp0("STOP ⊓ STOP"), events());
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)")
{
    check_process_initials(csp0("(a → STOP) ⊓ (b → STOP)"), events("τ"));
    check_process_afters(csp0("(a → STOP) ⊓ (b → STOP)"), event("τ"),
                         csp0s("a → STOP", "b → STOP"));
    check_process_afters(csp0("(a → STOP) ⊓ (b → STOP)"), event("a"), csp0s());
    check_process_traces_behavior(csp0("(a → STOP) ⊓ (b → STOP)"), events());
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}")
{
    check_process_initials(csp0("⊓ {a → STOP, b → STOP, c → STOP}"),
                           events("τ"));
    check_process_afters(csp0("⊓ {a → STOP, b → STOP, c → STOP}"), event("τ"),
                         csp0s("a → STOP", "b → STOP", "c → STOP"));
    check_process_afters(csp0("⊓ {a → STOP, b → STOP, c → STOP}"), event("a"),
                         csp0s());
    check_process_traces_behavior(csp0("⊓ {a → STOP, b → STOP, c → STOP}"),
                                  events());
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    check_process_initials(csp0("a → STOP"), events("a"));
    check_process_afters(csp0("a → STOP"), event("a"), csp0s("STOP"));
    check_process_afters(csp0("a → STOP"), event("b"), csp0s());
    check_process_traces_behavior(csp0("a → STOP"), events("a"));
}

TEST_CASE("a → b → STOP")
{
    check_process_initials(csp0("a → b → STOP"), events("a"));
    check_process_afters(csp0("a → b → STOP"), event("a"), csp0s("b → STOP"));
    check_process_afters(csp0("a → b → STOP"), event("b"), csp0s());
    check_process_traces_behavior(csp0("a → b → STOP"), events("a"));
}

TEST_CASE_GROUP("recursion");

TEST_CASE("let X=a → STOP within X")
{
    check_process_initials(csp0("let X=a → STOP within X"), events("a"));
    check_process_afters(csp0("let X=a → STOP within X"), event("a"),
                         csp0s("STOP"));
    check_process_traces_behavior(csp0("let X=a → STOP within X"), events("a"));
}

TEST_CASE("let X=a → Y Y=b → X within X")
{
    check_process_initials(csp0("let X=a → Y Y=b → X within X"), events("a"));
    check_process_afters(csp0("let X=a → Y Y=b → X within X"), event("a"),
                         csp0s("Y@0"));
    check_process_traces_behavior(csp0("let X=a → Y Y=b → X within X"),
                                  events("a"));
    check_process_sub_initials(csp0("let X=a → Y Y=b → X within X"),
                               csp0("Y@0"), events("b"));
    check_process_sub_afters(csp0("let X=a → Y Y=b → X within X"), csp0("Y@0"),
                             event("b"), csp0s("X@0"));
    check_process_sub_traces_behavior(csp0("let X=a → Y Y=b → X within X"),
                                      csp0("Y@0"), events("b"));
}

TEST_CASE_GROUP("sequential composition");

TEST_CASE("SKIP ; STOP")
{
    check_process_initials(csp0("SKIP ; STOP"), events("τ"));
    check_process_afters(csp0("SKIP ; STOP"), event("a"), csp0s());
    check_process_afters(csp0("SKIP ; STOP"), event("b"), csp0s());
    check_process_afters(csp0("SKIP ; STOP"), event("τ"), csp0s("STOP"));
    check_process_afters(csp0("SKIP ; STOP"), event("✔"), csp0s());
    check_process_traces_behavior(csp0("SKIP ; STOP"), events());
}

TEST_CASE("a → SKIP ; STOP")
{
    check_process_initials(csp0("a → SKIP ; STOP"), events("a"));
    check_process_afters(csp0("a → SKIP ; STOP"), event("a"),
                         csp0s("SKIP ; STOP"));
    check_process_afters(csp0("a → SKIP ; STOP"), event("b"), csp0s());
    check_process_afters(csp0("a → SKIP ; STOP"), event("τ"), csp0s());
    check_process_afters(csp0("a → SKIP ; STOP"), event("✔"), csp0s());
    check_process_traces_behavior(csp0("a → SKIP ; STOP"), events("a"));
}

TEST_CASE("(a → b → STOP □ SKIP) ; STOP")
{
    check_process_initials(csp0("(a → b → STOP □ SKIP) ; STOP"),
                           events("a", "τ"));
    check_process_afters(csp0("(a → b → STOP □ SKIP) ; STOP"), event("a"),
                         csp0s("b → STOP ; STOP"));
    check_process_afters(csp0("(a → b → STOP □ SKIP) ; STOP"), event("b"),
                         csp0s());
    check_process_afters(csp0("(a → b → STOP □ SKIP) ; STOP"), event("τ"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → b → STOP □ SKIP) ; STOP"), event("✔"),
                         csp0s());
    check_process_traces_behavior(csp0("(a → b → STOP □ SKIP) ; STOP"),
                                  events("a"));
}

TEST_CASE("(a → b → STOP ⊓ SKIP) ; STOP")
{
    check_process_initials(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), events("τ"));
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("a"),
                         csp0s());
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("b"),
                         csp0s());
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("τ"),
                         csp0s("a → b → STOP ; STOP", "SKIP ; STOP"));
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("✔"),
                         csp0s());
    check_process_traces_behavior(csp0("(a → b → STOP ⊓ SKIP) ; STOP"),
                                  events());
}
