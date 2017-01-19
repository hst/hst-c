/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "operators.h"

#include <assert.h>

#include "ccan/container_of/container_of.h"
#include "basics.h"
#include "behavior.h"
#include "environment.h"
#include "event.h"
#include "id-set.h"
#include "process.h"
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

#define MAX_NAME_LENGTH 4096

struct csp_collect_name {
    struct csp_name_visitor visitor;
    char name[MAX_NAME_LENGTH];
    char *cursor;
    size_t bytes_remaining;
};

static void
csp_collect_name_visit(struct csp *csp, struct csp_name_visitor *visitor,
                       const char *str, size_t length)
{
    struct csp_collect_name *self =
            container_of(visitor, struct csp_collect_name, visitor);
    assert(length < self->bytes_remaining);
    memcpy(self->cursor, str, length);
    self->cursor += length;
    self->bytes_remaining -= length;
    *self->cursor = '\0';
}

static void
csp_collect_name_init(struct csp_collect_name *self)
{
    self->visitor.visit = csp_collect_name_visit;
    self->cursor = self->name;
    self->bytes_remaining = MAX_NAME_LENGTH;
}

/* Verify the name of the given CSP₀ process. */
static void
check_process_name_(const char *filename, unsigned int line,
                    struct csp_process_factory process_, const char *expected)
{
    struct csp *csp;
    struct csp_process *process;
    struct csp_collect_name collect;
    check_alloc(csp, csp_new());
    process = csp_process_factory_create(csp, process_);
    csp_collect_name_init(&collect);
    csp_process_name(csp, process, &collect.visitor);
    check_streq_(filename, line, collect.name, expected);
    csp_free(csp);
}
#define check_process_name ADD_FILE_AND_LINE(check_process_name_)

/* Verify the `initials` of the given CSP₀ process. */
static void
check_process_initials_(const char *filename, unsigned int line,
                        struct csp_process_factory process_,
                        struct csp_event_set_factory expected_initials_)
{
    struct csp *csp;
    struct csp_process *process;
    struct csp_event_set actual;
    struct csp_collect_events collect = csp_collect_events(&actual);
    check_alloc(csp, csp_new());
    csp_event_set_init(&actual);
    process = csp_process_factory_create(csp, process_);
    csp_process_visit_initials(csp, process, &collect.visitor);
    check_event_set_eq_(filename, line, &actual,
                        csp_event_set_factory_create(csp, expected_initials_));
    csp_event_set_done(&actual);
    csp_free(csp);
}
#define check_process_initials ADD_FILE_AND_LINE(check_process_initials_)

/* Verify the `afters` of the given CSP₀ process after performing `initial`. */
static void
check_process_afters_(const char *filename, unsigned int line,
                      struct csp_process_factory process_,
                      struct csp_event_factory initial_,
                      struct csp_process_set_factory expected_afters_)
{
    struct csp *csp;
    struct csp_process *process;
    const struct csp_event *initial;
    struct csp_process_set actual;
    struct csp_collect_afters collect = csp_collect_afters(&actual);
    check_alloc(csp, csp_new());
    csp_process_set_init(&actual);
    process = csp_process_factory_create(csp, process_);
    initial = csp_event_factory_create(csp, initial_);
    csp_process_visit_afters(csp, process, initial, &collect.visitor);
    check_process_set_eq_(
            filename, line, csp, &actual,
            csp_process_set_factory_create(csp, expected_afters_));
    csp_process_set_done(&actual);
    csp_free(csp);
}
#define check_process_afters ADD_FILE_AND_LINE(check_process_afters_)

/* Verify all of the subprocesses that are reachable from `process`. */
static void
check_process_reachable_(const char *filename, unsigned int line,
                         struct csp_process_factory process_,
                         struct csp_process_set_factory expected_reachable_)
{
    struct csp *csp;
    struct csp_process *process;
    struct csp_process_set actual;
    struct csp_collect_processes collect = csp_collect_processes(&actual);
    check_alloc(csp, csp_new());
    process = csp_process_factory_create(csp, process_);
    csp_process_set_init(&actual);
    csp_process_bfs(csp, process, &collect.visitor);
    check_process_set_eq_(
            filename, line, csp, &actual,
            csp_process_set_factory_create(csp, expected_reachable_));
    csp_process_set_done(&actual);
    csp_free(csp);
}
#define check_process_reachable ADD_FILE_AND_LINE(check_process_reachable_)

/* Verify the traces behavior of the given CSP₀ process. */
static void
check_process_traces_behavior_(const char *filename, unsigned int line,
                               struct csp_process_factory process_,
                               struct csp_event_set_factory expected_initials_)
{
    struct csp *csp;
    struct csp_process *process;
    const struct csp_event_set *expected_initials;
    struct csp_behavior behavior;
    check_alloc(csp, csp_new());
    csp_behavior_init(&behavior);
    process = csp_process_factory_create(csp, process_);
    expected_initials = csp_event_set_factory_create(csp, expected_initials_);
    csp_process_get_behavior(csp, process, CSP_TRACES, &behavior);
    check_event_set_eq_(filename, line, &behavior.initials, expected_initials);
    csp_behavior_done(&behavior);
    csp_free(csp);
}
#define check_process_traces_behavior \
    ADD_FILE_AND_LINE(check_process_traces_behavior_)

/* Verify the `initials` of a subprocess.  `subprocess` should be a process that
 * has been defined as part of `process`. */
static void
check_process_sub_initials_(const char *filename, unsigned int line,
                            struct csp_process_factory process_,
                            struct csp_process_factory subprocess_,
                            struct csp_event_set_factory expected_initials_)
{
    struct csp *csp;
    UNNEEDED struct csp_process *process;
    struct csp_process *subprocess;
    struct csp_event_set actual;
    struct csp_collect_events collect = csp_collect_events(&actual);
    check_alloc(csp, csp_new());
    csp_event_set_init(&actual);
    process = csp_process_factory_create(csp, process_);
    subprocess = csp_process_factory_create(csp, subprocess_);
    csp_process_visit_initials(csp, subprocess, &collect.visitor);
    check_event_set_eq_(filename, line, &actual,
                        csp_event_set_factory_create(csp, expected_initials_));
    csp_event_set_done(&actual);
    csp_free(csp);
}
#define check_process_sub_initials \
    ADD_FILE_AND_LINE(check_process_sub_initials_)

/* Verify the `afters` of a subprocess after performing `initial`.  `subprocess`
 * should be a process that has been defined as part of `process`. */
static void
check_process_sub_afters_(const char *filename, unsigned int line,
                          struct csp_process_factory process_,
                          struct csp_process_factory subprocess_,
                          struct csp_event_factory initial_,
                          struct csp_process_set_factory expected_afters_)
{
    struct csp *csp;
    UNNEEDED struct csp_process *process;
    struct csp_process *subprocess;
    const struct csp_event *initial;
    struct csp_process_set actual;
    struct csp_collect_afters collect = csp_collect_afters(&actual);
    check_alloc(csp, csp_new());
    csp_process_set_init(&actual);
    process = csp_process_factory_create(csp, process_);
    subprocess = csp_process_factory_create(csp, subprocess_);
    initial = csp_event_factory_create(csp, initial_);
    csp_process_visit_afters(csp, subprocess, initial, &collect.visitor);
    check_process_set_eq_(
            filename, line, csp, &actual,
            csp_process_set_factory_create(csp, expected_afters_));
    csp_process_set_done(&actual);
    csp_free(csp);
}
#define check_process_sub_afters ADD_FILE_AND_LINE(check_process_sub_afters_)

/* Verify the traces behavior of the given CSP₀ process.  `subprocess` should be
 * a process that has been defined as part of `process`. */
static void
check_process_sub_traces_behavior_(
        const char *filename, unsigned int line,
        struct csp_process_factory process_,
        struct csp_process_factory subprocess_,
        struct csp_event_set_factory expected_initials_)
{
    struct csp *csp;
    UNNEEDED struct csp_process *process;
    struct csp_process *subprocess;
    const struct csp_event_set *expected_initials;
    struct csp_behavior behavior;
    check_alloc(csp, csp_new());
    csp_behavior_init(&behavior);
    process = csp_process_factory_create(csp, process_);
    subprocess = csp_process_factory_create(csp, subprocess_);
    expected_initials = csp_event_set_factory_create(csp, expected_initials_);
    csp_process_get_behavior(csp, subprocess, CSP_TRACES, &behavior);
    check_event_set_eq_(filename, line, &behavior.initials, expected_initials);
    csp_behavior_done(&behavior);
    csp_free(csp);
}
#define check_process_sub_traces_behavior \
    ADD_FILE_AND_LINE(check_process_sub_traces_behavior_)

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP")
{
    check_process_name(csp0("STOP □ STOP"), "□ {STOP}");
    check_process_initials(csp0("STOP □ STOP"), events());
    check_process_afters(csp0("STOP □ STOP"), event("a"), csp0s());
    check_process_reachable(csp0("STOP □ STOP"), csp0s("STOP □ STOP"));
    check_process_traces_behavior(csp0("STOP □ STOP"), events());
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)")
{
    check_process_name(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"),
                       "a → STOP □ (b → STOP ⊓ c → STOP)");
    check_process_initials(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"),
                           events("a", "τ"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"), event("a"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"), event("b"),
                         csp0s());
    check_process_afters(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"), event("τ"),
                         csp0s("a → STOP □ b → STOP", "a → STOP □ c → STOP"));
    check_process_reachable(
            csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"),
            csp0s("(a → STOP) □ (b → STOP ⊓ c → STOP)", "a → STOP □ b → STOP",
                  "a → STOP □ c → STOP", "STOP"));
    check_process_traces_behavior(csp0("(a → STOP) □ (b → STOP ⊓ c → STOP)"),
                                  events("a"));
}

TEST_CASE("(a → STOP) □ (b → STOP)")
{
    check_process_name(csp0("(a → STOP) □ (b → STOP)"), "a → STOP □ b → STOP");
    check_process_initials(csp0("(a → STOP) □ (b → STOP)"), events("a", "b"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP)"), event("a"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP)"), event("b"),
                         csp0s("STOP"));
    check_process_afters(csp0("(a → STOP) □ (b → STOP)"), event("τ"), csp0s());
    check_process_reachable(csp0("(a → STOP) □ (b → STOP)"),
                            csp0s("(a → STOP) □ (b → STOP)", "STOP"));
    check_process_traces_behavior(csp0("(a → STOP) □ (b → STOP)"),
                                  events("a", "b"));
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}")
{
    check_process_name(csp0("□ {a → STOP, b → STOP, c → STOP}"),
                       "□ {a → STOP, b → STOP, c → STOP}");
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
    check_process_reachable(csp0("□ {a → STOP, b → STOP, c → STOP}"),
                            csp0s("□ {a → STOP, b → STOP, c → STOP}", "STOP"));
    check_process_traces_behavior(csp0("□ {a → STOP, b → STOP, c → STOP}"),
                                  events("a", "b", "c"));
}

TEST_CASE_GROUP("interleaving");

TEST_CASE("STOP ⫴ STOP")
{
    check_process_name(csp0("STOP ⫴ STOP"), "⫴ {STOP}");
    check_process_initials(csp0("STOP ⫴ STOP"), events("✔"));
    check_process_afters(csp0("STOP ⫴ STOP"), event("✔"), csp0s("STOP"));
    check_process_afters(csp0("STOP ⫴ STOP"), event("a"), csp0s());
    check_process_afters(csp0("STOP ⫴ STOP"), event("τ"), csp0s());
    check_process_reachable(csp0("STOP ⫴ STOP"), csp0s("STOP ⫴ STOP", "STOP"));
    check_process_traces_behavior(csp0("STOP ⫴ STOP"), events("✔"));
}

TEST_CASE("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)")
{
    check_process_name(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"),
                       "a → STOP ⫴ b → STOP ⊓ c → STOP");
    check_process_initials(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"),
                           events("a", "τ"));
    check_process_afters(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"), event("a"),
                         csp0s("STOP ⫴ (b → STOP ⊓ c → STOP)"));
    check_process_afters(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"), event("b"),
                         csp0s());
    check_process_afters(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"), event("τ"),
                         csp0s("a → STOP ⫴ b → STOP", "a → STOP ⫴ c → STOP"));
    check_process_reachable(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"),
                            csp0s("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)",
                                  "STOP ⫴ (b → STOP ⊓ c → STOP)",
                                  "STOP ⫴ b → STOP", "STOP ⫴ c → STOP",
                                  "a → STOP ⫴ b → STOP", "a → STOP ⫴ c → STOP",
                                  "a → STOP ⫴ STOP", "STOP ⫴ STOP", "STOP"));
    check_process_traces_behavior(csp0("(a → STOP) ⫴ (b → STOP ⊓ c → STOP)"),
                                  events("a"));
}

TEST_CASE("a → STOP ⫴ b → STOP")
{
    check_process_name(csp0("a → STOP ⫴ b → STOP"), "a → STOP ⫴ b → STOP");
    check_process_initials(csp0("a → STOP ⫴ b → STOP"), events("a", "b"));
    check_process_afters(csp0("a → STOP ⫴ b → STOP"), event("a"),
                         csp0s("STOP ⫴ b → STOP"));
    check_process_afters(csp0("a → STOP ⫴ b → STOP"), event("b"),
                         csp0s("a → STOP ⫴ STOP"));
    check_process_afters(csp0("a → STOP ⫴ b → STOP"), event("τ"), csp0s());
    check_process_reachable(csp0("a → STOP ⫴ b → STOP"),
                            csp0s("a → STOP ⫴ b → STOP", "a → STOP ⫴ STOP",
                                  "STOP ⫴ b → STOP", "STOP ⫴ STOP", "STOP"));
    check_process_traces_behavior(csp0("a → STOP ⫴ b → STOP"),
                                  events("a", "b"));
}

TEST_CASE("a → SKIP ⫴ b → SKIP")
{
    check_process_name(csp0("a → SKIP ⫴ b → SKIP"), "a → SKIP ⫴ b → SKIP");
    check_process_initials(csp0("a → SKIP ⫴ b → SKIP"), events("a", "b"));
    check_process_afters(csp0("a → SKIP ⫴ b → SKIP"), event("a"),
                         csp0s("SKIP ⫴ b → SKIP"));
    check_process_afters(csp0("a → SKIP ⫴ b → SKIP"), event("b"),
                         csp0s("a → SKIP ⫴ SKIP"));
    check_process_afters(csp0("a → SKIP ⫴ b → SKIP"), event("τ"), csp0s());
    check_process_afters(csp0("a → SKIP ⫴ b → SKIP"), event("✔"), csp0s());
    check_process_reachable(
            csp0("a → SKIP ⫴ b → SKIP"),
            csp0s("a → SKIP ⫴ b → SKIP", "a → SKIP ⫴ SKIP", "a → SKIP ⫴ STOP",
                  "SKIP ⫴ b → SKIP", "STOP ⫴ b → SKIP", "STOP ⫴ SKIP",
                  "STOP ⫴ STOP", "SKIP ⫴ SKIP", "STOP"));
    check_process_traces_behavior(csp0("a → SKIP ⫴ b → SKIP"),
                                  events("a", "b"));
}

TEST_CASE("(a → SKIP ⫴ b → SKIP) ; c → STOP")
{
    check_process_name(csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"),
                       "(a → SKIP ⫴ b → SKIP) ; c → STOP");
    check_process_initials(csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"),
                           events("a", "b"));
    check_process_afters(csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"), event("a"),
                         csp0s("(SKIP ⫴ b → SKIP) ; c → STOP"));
    check_process_afters(csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"), event("b"),
                         csp0s("(a → SKIP ⫴ SKIP) ; c → STOP"));
    check_process_afters(csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"), event("τ"),
                         csp0s());
    check_process_reachable(
            csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"),
            csp0s("(a → SKIP ⫴ b → SKIP) ; c → STOP",
                  "(a → SKIP ⫴ SKIP) ; c → STOP",
                  "(a → SKIP ⫴ STOP) ; c → STOP",
                  "(SKIP ⫴ b → SKIP) ; c → STOP",
                  "(STOP ⫴ b → SKIP) ; c → STOP", "(STOP ⫴ SKIP) ; c → STOP",
                  "(STOP ⫴ STOP) ; c → STOP", "(SKIP ⫴ SKIP) ; c → STOP",
                  "c → STOP", "STOP"));
    check_process_traces_behavior(csp0("(a → SKIP ⫴ b → SKIP) ; c → STOP"),
                                  events("a", "b"));
}

TEST_CASE("⫴ {a → STOP, b → STOP, c → STOP}")
{
    check_process_name(csp0("⫴ {a → STOP, b → STOP, c → STOP}"),
                       "⫴ {a → STOP, b → STOP, c → STOP}");
    check_process_initials(csp0("⫴ {a → STOP, b → STOP, c → STOP}"),
                           events("a", "b", "c"));
    check_process_afters(csp0("⫴ {a → STOP, b → STOP, c → STOP}"), event("a"),
                         csp0s("⫴ {STOP, b → STOP, c → STOP}"));
    check_process_afters(csp0("⫴ {a → STOP, b → STOP, c → STOP}"), event("b"),
                         csp0s("⫴ {a → STOP, STOP, c → STOP}"));
    check_process_afters(csp0("⫴ {a → STOP, b → STOP, c → STOP}"), event("c"),
                         csp0s("⫴ {a → STOP, b → STOP, STOP}"));
    check_process_afters(csp0("⫴ {a → STOP, b → STOP, c → STOP}"), event("τ"),
                         csp0s());
    check_process_reachable(csp0("⫴ {a → STOP, b → STOP, c → STOP}"),
                            csp0s("⫴ {a → STOP, b → STOP, c → STOP}",
                                  "⫴ {STOP, b → STOP, c → STOP}",
                                  "⫴ {a → STOP, STOP, c → STOP}",
                                  "⫴ {a → STOP, b → STOP, STOP}",
                                  "⫴ {a → STOP, STOP}", "⫴ {b → STOP, STOP}",
                                  "⫴ {c → STOP, STOP}", "⫴ {STOP}", "STOP"));
    check_process_traces_behavior(csp0("⫴ {a → STOP, b → STOP, c → STOP}"),
                                  events("a", "b", "c"));
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP")
{
    check_process_name(csp0("STOP ⊓ STOP"), "⊓ {STOP}");
    check_process_initials(csp0("STOP ⊓ STOP"), events("τ"));
    check_process_afters(csp0("STOP ⊓ STOP"), event("τ"), csp0s("STOP"));
    check_process_afters(csp0("STOP ⊓ STOP"), event("a"), csp0s());
    check_process_reachable(csp0("STOP ⊓ STOP"), csp0s("STOP ⊓ STOP", "STOP"));
    check_process_traces_behavior(csp0("STOP ⊓ STOP"), events());
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)")
{
    check_process_name(csp0("(a → STOP) ⊓ (b → STOP)"), "a → STOP ⊓ b → STOP");
    check_process_initials(csp0("(a → STOP) ⊓ (b → STOP)"), events("τ"));
    check_process_afters(csp0("(a → STOP) ⊓ (b → STOP)"), event("τ"),
                         csp0s("a → STOP", "b → STOP"));
    check_process_afters(csp0("(a → STOP) ⊓ (b → STOP)"), event("a"), csp0s());
    check_process_reachable(
            csp0("(a → STOP) ⊓ (b → STOP)"),
            csp0s("(a → STOP) ⊓ (b → STOP)", "a → STOP", "b → STOP", "STOP"));
    check_process_traces_behavior(csp0("(a → STOP) ⊓ (b → STOP)"), events());
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}")
{
    check_process_name(csp0("⊓ {a → STOP, b → STOP, c → STOP}"),
                       "⊓ {a → STOP, b → STOP, c → STOP}");
    check_process_initials(csp0("⊓ {a → STOP, b → STOP, c → STOP}"),
                           events("τ"));
    check_process_afters(csp0("⊓ {a → STOP, b → STOP, c → STOP}"), event("τ"),
                         csp0s("a → STOP", "b → STOP", "c → STOP"));
    check_process_afters(csp0("⊓ {a → STOP, b → STOP, c → STOP}"), event("a"),
                         csp0s());
    check_process_reachable(csp0("⊓ {a → STOP, b → STOP, c → STOP}"),
                            csp0s("⊓ {a → STOP, b → STOP, c → STOP}",
                                  "a → STOP", "b → STOP", "c → STOP", "STOP"));
    check_process_traces_behavior(csp0("⊓ {a → STOP, b → STOP, c → STOP}"),
                                  events());
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP")
{
    check_process_name(csp0("a → STOP"), "a → STOP");
    check_process_initials(csp0("a → STOP"), events("a"));
    check_process_afters(csp0("a → STOP"), event("a"), csp0s("STOP"));
    check_process_afters(csp0("a → STOP"), event("b"), csp0s());
    check_process_reachable(csp0("a → STOP"), csp0s("a → STOP", "STOP"));
    check_process_traces_behavior(csp0("a → STOP"), events("a"));
}

TEST_CASE("a → b → STOP")
{
    check_process_name(csp0("a → b → STOP"), "a → b → STOP");
    check_process_initials(csp0("a → b → STOP"), events("a"));
    check_process_afters(csp0("a → b → STOP"), event("a"), csp0s("b → STOP"));
    check_process_afters(csp0("a → b → STOP"), event("b"), csp0s());
    check_process_reachable(csp0("a → b → STOP"),
                            csp0s("a → b → STOP", "b → STOP", "STOP"));
    check_process_traces_behavior(csp0("a → b → STOP"), events("a"));
}

TEST_CASE_GROUP("recursion");

TEST_CASE("let X=a → STOP within X")
{
    check_process_name(csp0("let X=a → STOP within X"),
                       "let X=a → STOP within X");
    check_process_initials(csp0("let X=a → STOP within X"), events("a"));
    check_process_afters(csp0("let X=a → STOP within X"), event("a"),
                         csp0s("STOP"));
    check_process_traces_behavior(csp0("let X=a → STOP within X"), events("a"));
    check_process_reachable(csp0("let X=a → STOP within X"),
                            csp0s("X@0", "STOP"));
}

TEST_CASE("let X=a → Y Y=b → X within X")
{
    check_process_name(csp0("let X=a → Y Y=b → X within X"),
                       "let X=a → Y Y=b → X within X");
    check_process_initials(csp0("let X=a → Y Y=b → X within X"), events("a"));
    check_process_afters(csp0("let X=a → Y Y=b → X within X"), event("a"),
                         csp0s("Y@0"));
    check_process_traces_behavior(csp0("let X=a → Y Y=b → X within X"),
                                  events("a"));
    check_process_reachable(csp0("let X=a → Y Y=b → X within X"),
                            csp0s("X@0", "Y@0"));
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
    check_process_name(csp0("SKIP ; STOP"), "SKIP ; STOP");
    check_process_initials(csp0("SKIP ; STOP"), events("τ"));
    check_process_afters(csp0("SKIP ; STOP"), event("a"), csp0s());
    check_process_afters(csp0("SKIP ; STOP"), event("b"), csp0s());
    check_process_afters(csp0("SKIP ; STOP"), event("τ"), csp0s("STOP"));
    check_process_afters(csp0("SKIP ; STOP"), event("✔"), csp0s());
    check_process_reachable(csp0("SKIP ; STOP"), csp0s("SKIP ; STOP", "STOP"));
    check_process_traces_behavior(csp0("SKIP ; STOP"), events());
}

TEST_CASE("a → SKIP ; STOP")
{
    check_process_name(csp0("a → SKIP ; STOP"), "a → SKIP ; STOP");
    check_process_initials(csp0("a → SKIP ; STOP"), events("a"));
    check_process_afters(csp0("a → SKIP ; STOP"), event("a"),
                         csp0s("SKIP ; STOP"));
    check_process_afters(csp0("a → SKIP ; STOP"), event("b"), csp0s());
    check_process_afters(csp0("a → SKIP ; STOP"), event("τ"), csp0s());
    check_process_afters(csp0("a → SKIP ; STOP"), event("✔"), csp0s());
    check_process_reachable(csp0("a → SKIP ; STOP"),
                            csp0s("a → SKIP ; STOP", "SKIP ; STOP", "STOP"));
    check_process_traces_behavior(csp0("a → SKIP ; STOP"), events("a"));
}

TEST_CASE("(a → b → STOP □ SKIP) ; STOP")
{
    check_process_name(csp0("(a → b → STOP □ SKIP) ; STOP"),
                       "(SKIP □ a → b → STOP) ; STOP");
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
    check_process_reachable(csp0("(a → b → STOP □ SKIP) ; STOP"),
                            csp0s("(a → b → STOP □ SKIP) ; STOP",
                                  "b → STOP ; STOP", "STOP ; STOP", "STOP"));
    check_process_traces_behavior(csp0("(a → b → STOP □ SKIP) ; STOP"),
                                  events("a"));
}

TEST_CASE("(a → b → STOP ⊓ SKIP) ; STOP")
{
    check_process_name(csp0("(a → b → STOP ⊓ SKIP) ; STOP"),
                       "(SKIP ⊓ a → b → STOP) ; STOP");
    check_process_initials(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), events("τ"));
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("a"),
                         csp0s());
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("b"),
                         csp0s());
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("τ"),
                         csp0s("a → b → STOP ; STOP", "SKIP ; STOP"));
    check_process_afters(csp0("(a → b → STOP ⊓ SKIP) ; STOP"), event("✔"),
                         csp0s());
    check_process_reachable(
            csp0("(a → b → STOP ⊓ SKIP) ; STOP"),
            csp0s("(a → b → STOP ⊓ SKIP) ; STOP", "a → b → STOP ; STOP",
                  "SKIP ; STOP", "b → STOP ; STOP", "STOP ; STOP", "STOP"));
    check_process_traces_behavior(csp0("(a → b → STOP ⊓ SKIP) ; STOP"),
                                  events());
}
