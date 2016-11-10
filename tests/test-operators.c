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

/* The test cases in this file verify that we've implemented each of the CSP
 * operators correctly: specifically, that they have the right "initials" and
 * "afters" sets, as defined by CSP's operational semantics.
 *
 * We've provided some helper macros that make these test cases easy to write.
 * In particular, you can assume that the CSP₀ parser works as expected; that
 * will have been checked in test-csp0.c.
 */

/* Verify the `initials` of the given CSP₀ process.  `events` should be a
 * (possibly empty) parenthesized list of event names. */
#define check_csp0_initials(csp0, events) \
    do { \
        struct csp  *csp; \
        csp_id  __process; \
        struct csp_id_set_builder  __builder; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
        check_alloc(csp, csp_new()); \
        csp_id_set_builder_init(&__builder); \
        csp_id_set_init(&__actual); \
        csp_id_set_init(&__expected); \
        check0(csp_load_csp0_string(csp, (csp0), &__process)); \
        csp_process_build_initials(csp, __process, &__builder); \
        csp_id_set_build(&__actual, &__builder); \
        fill_event_id_set(&__expected, events); \
        check_with_msg( \
                csp_id_set_eq(&__actual, &__expected), \
                "initials(" csp0 ") == {" \
                CPPMAGIC_JOIN(",", CPPMAGIC_UNPACK(events)) \
                "}"); \
        csp_id_set_builder_done(&__builder); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
        csp_free(csp); \
    } while (0)

/* Verify the `afters` of the given CSP₀ process after performing `initial`.
 * `initial` should be an event name.  `afters` should be a (possibly empty)
 * parenthesized list of CSP₀ processes. */
#define check_csp0_afters(csp0, initial, afters) \
    do { \
        struct csp  *csp; \
        csp_id  __process; \
        csp_id  __initial; \
        struct csp_id_set_builder  __builder; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
        check_alloc(csp, csp_new()); \
        csp_id_set_builder_init(&__builder); \
        csp_id_set_init(&__actual); \
        csp_id_set_init(&__expected); \
        check0(csp_load_csp0_string(csp, (csp0), &__process)); \
        __initial = csp_get_event_id(csp, (initial)); \
        csp_process_build_afters(csp, __process, __initial, &__builder); \
        csp_id_set_build(&__actual, &__builder); \
        fill_csp0_set(&__expected, afters); \
        check_with_msg( \
                csp_id_set_eq(&__actual, &__expected), \
                "afters(" csp0 ", " initial ") == {" \
                CPPMAGIC_JOIN(", ", CPPMAGIC_UNPACK(afters)) \
                "}"); \
        csp_id_set_builder_done(&__builder); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
        csp_free(csp); \
    } while (0)

/* Verify the `initials` of a subprocess.  `subprocess` should be a process that
 * has been defined as part of `csp0`.  `events` should be a (possibly empty)
 * parenthesized list of event names. */
#define check_csp0_sub_initials(csp0, subprocess, events) \
    do { \
        struct csp  *csp; \
        csp_id  __process; \
        csp_id  __subprocess; \
        struct csp_id_set_builder  __builder; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
        check_alloc(csp, csp_new()); \
        csp_id_set_builder_init(&__builder); \
        csp_id_set_init(&__actual); \
        csp_id_set_init(&__expected); \
        check0(csp_load_csp0_string(csp, (csp0), &__process)); \
        check0(csp_load_csp0_string(csp, (subprocess), &__subprocess)); \
        csp_process_build_initials(csp, __subprocess, &__builder); \
        csp_id_set_build(&__actual, &__builder); \
        fill_event_id_set(&__expected, events); \
        check_with_msg( \
                csp_id_set_eq(&__actual, &__expected), \
                "initials(" subprocess ") == {" \
                CPPMAGIC_JOIN(",", CPPMAGIC_UNPACK(events)) \
                "}"); \
        csp_id_set_builder_done(&__builder); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
        csp_free(csp); \
    } while (0)

/* Verify the `afters` of a subprocess after performing `initial`.  `subprocess`
 * should be a process that has been defined as part of `csp0`.  `initial`
 * should be an event name.  `afters` should be a (possibly empty) parenthesized
 * list of CSP₀ processes. */
#define check_csp0_sub_afters(csp0, subprocess, initial, afters) \
    do { \
        struct csp  *csp; \
        csp_id  __process; \
        csp_id  __subprocess; \
        csp_id  __initial; \
        struct csp_id_set_builder  __builder; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
        check_alloc(csp, csp_new()); \
        csp_id_set_builder_init(&__builder); \
        csp_id_set_init(&__actual); \
        csp_id_set_init(&__expected); \
        check0(csp_load_csp0_string(csp, (csp0), &__process)); \
        check0(csp_load_csp0_string(csp, (subprocess), &__subprocess)); \
        __initial = csp_get_event_id(csp, (initial)); \
        csp_process_build_afters(csp, __subprocess, __initial, &__builder); \
        csp_id_set_build(&__actual, &__builder); \
        fill_csp0_set(&__expected, afters); \
        check_with_msg( \
                csp_id_set_eq(&__actual, &__expected), \
                "afters(" subprocess ", " initial ") == {" \
                CPPMAGIC_JOIN(", ", CPPMAGIC_UNPACK(afters)) \
                "}"); \
        csp_id_set_builder_done(&__builder); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
        csp_free(csp); \
    } while (0)

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP") {
    check_csp0_initials("STOP □ STOP", ());
    check_csp0_afters("STOP □ STOP", "a", ());
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)") {
    check_csp0_initials("(a → STOP) □ (b → STOP ⊓ c → STOP)", ("a", "τ"));
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "a", ("STOP"));
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "b", ());
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "τ",
                      ("a → STOP □ b → STOP", "a → STOP □ c → STOP"));
}

TEST_CASE("(a → STOP) □ (b → STOP)") {
    check_csp0_initials("(a → STOP) □ (b → STOP)", ("a", "b"));
    check_csp0_afters("(a → STOP) □ (b → STOP)", "a", ("STOP"));
    check_csp0_afters("(a → STOP) □ (b → STOP)", "b", ("STOP"));
    check_csp0_afters("(a → STOP) □ (b → STOP)", "τ", ());
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}") {
    check_csp0_initials("□ {a → STOP, b → STOP, c → STOP}", ("a", "b", "c"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "a", ("STOP"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "b", ("STOP"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "c", ("STOP"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "τ", ());
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP") {
    check_csp0_initials("STOP ⊓ STOP", ("τ"));
    check_csp0_afters("STOP ⊓ STOP", "τ", ("STOP"));
    check_csp0_afters("STOP ⊓ STOP", "a", ());
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)") {
    check_csp0_initials("(a → STOP) ⊓ (b → STOP)", ("τ"));
    check_csp0_afters("(a → STOP) ⊓ (b → STOP)", "τ", ("a → STOP", "b → STOP"));
    check_csp0_afters("(a → STOP) ⊓ (b → STOP)", "a", ());
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}") {
    check_csp0_initials("⊓ {a → STOP, b → STOP, c → STOP}", ("τ"));
    check_csp0_afters("⊓ {a → STOP, b → STOP, c → STOP}", "τ",
                      ("a → STOP", "b → STOP", "c → STOP"));
    check_csp0_afters("⊓ {a → STOP, b → STOP, c → STOP}", "a", ());
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP") {
    check_csp0_initials("a → STOP", ("a"));
    check_csp0_afters("a → STOP", "a", ("STOP"));
    check_csp0_afters("a → STOP", "b", ());
}

TEST_CASE("a → b → STOP") {
    check_csp0_initials("a → b → STOP", ("a"));
    check_csp0_afters("a → b → STOP", "a", ("b → STOP"));
    check_csp0_afters("a → b → STOP", "b", ());
}

TEST_CASE_GROUP("recursion");

TEST_CASE("let X=a → STOP within X") {
    check_csp0_initials("let X=a → STOP within X", ("a"));
    check_csp0_afters("let X=a → STOP within X", "a", ("STOP"));
}

TEST_CASE("let X=a → Y Y=b → X within X") {
    check_csp0_initials("let X=a → Y Y=b → X within X", ("a"));
    check_csp0_afters("let X=a → Y Y=b → X within X", "a", ("Y@0"));
    check_csp0_sub_initials("let X=a → Y Y=b → X within X", "Y@0", ("b"));
    check_csp0_sub_afters("let X=a → Y Y=b → X within X", "Y@0", "b", ("X@0"));
}

TEST_CASE_GROUP("sequential composition");

TEST_CASE("SKIP ; STOP") {
    check_csp0_initials("SKIP ; STOP", ("τ"));
    check_csp0_afters("SKIP ; STOP", "a", ());
    check_csp0_afters("SKIP ; STOP", "b", ());
    check_csp0_afters("SKIP ; STOP", "τ", ("STOP"));
    check_csp0_afters("SKIP ; STOP", "✔", ());
}

TEST_CASE("a → SKIP ; STOP") {
    check_csp0_initials("a → SKIP ; STOP", ("a"));
    check_csp0_afters("a → SKIP ; STOP", "a", ("SKIP ; STOP"));
    check_csp0_afters("a → SKIP ; STOP", "b", ());
    check_csp0_afters("a → SKIP ; STOP", "τ", ());
    check_csp0_afters("a → SKIP ; STOP", "✔", ());
}

TEST_CASE("(a → b → STOP □ SKIP) ; STOP") {
    check_csp0_initials("(a → b → STOP □ SKIP) ; STOP", ("a", "τ"));
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "a", ("b → STOP ; STOP"));
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "b", ());
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "τ", ("STOP"));
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "✔", ());
}

TEST_CASE("(a → b → STOP ⊓ SKIP) ; STOP") {
    check_csp0_initials("(a → b → STOP ⊓ SKIP) ; STOP", ("τ"));
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "a", ());
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "b", ());
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "τ",
                      ("a → b → STOP ; STOP", "SKIP ; STOP"));
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "✔", ());
}
