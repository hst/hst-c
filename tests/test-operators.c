/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "hst.h"
#include "test-case-harness.h"

/* The test cases in this file verify that we've implemented each of the CSP
 * operators correctly: specifically, that they have the right "initials" and
 * "afters" sets, as defined by CSP's operational semantics.
 *
 * We've provided some helper macros that make these test cases easy to write.
 * In particular, you can assume that the CSP₀ parser works as expected; that
 * will have been checked in test-csp0.c.
 */

/**
 * CPPMAGIC_MAP_SEMICOLONS - iterate another macro across arguments
 * @m: name of a one argument macro
 *
 * CPPMAGIC_MAP_SEMICOLONS(@m, @a1, @a2, ... @an)
 *     expands to the expansion of @m(@a1) ; @m(@a2) ; ... ; @m(@an)
 */
#define _CPPMAGIC_MAP_SEMICOLONS_()  _CPPMAGIC_MAP_SEMICOLONS
#define _CPPMAGIC_MAP_SEMICOLONS(m_, a_, ...) \
    m_(a_) \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
        (; CPPMAGIC_DEFER2(_CPPMAGIC_MAP_SEMICOLONS_)()(m_, __VA_ARGS__)) \
        ()
#define CPPMAGIC_MAP_SEMICOLONS(m_, ...) \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
        (CPPMAGIC_EVAL(_CPPMAGIC_MAP_SEMICOLONS(m_, __VA_ARGS__))) \
        ()

/* Takes a parenthesized list and removes the parentheses. */
#define CPPMAGIC_UNPACK(args) CPPMAGIC_UNPACK_ args
#define CPPMAGIC_UNPACK_(...) __VA_ARGS__

/* Add IDs to a set.  `elements` should be a (possibly empty) parenthesized list
 * of "elements".  `id_adder` should be a macro that takes in one of those
 * elements, translates it into an ID, and adds that ID to an ID set builder
 * named `__builder`. */
#define fill_id_set(id_adder, set, elements) \
    do { \
        struct csp_id_set_builder  __builder; \
        csp_id_set_builder_init(&__builder); \
        CPPMAGIC_MAP_SEMICOLONS(id_adder, CPPMAGIC_UNPACK(elements)); \
        csp_id_set_build((set), &__builder); \
        csp_id_set_builder_done(&__builder); \
    } while (0)

/* Adds events to a set.  `events` should be a (possibly empty) parenthesized
 * list of event names.  We automatically translate those event names into event
 * IDs. */
#define fill_event_id_set(set, events) \
    fill_id_set(add_event_id, set, events)
#define add_event_id(event_name) \
    do { \
        csp_id  __event = csp_get_event_id(csp, (event_name)); \
        csp_id_set_builder_add(&__builder, __event); \
    } while (0)

/* Adds processes to a cset.  `processes` should be a (possibly empty)
 * parenthesized list of CSP₀ process descriptions.  We automatically parse
 * those into process IDs and add new references to those processes to `set`. */
#define fill_csp0_set(set, processes) \
    fill_id_set(add_csp0_process, set, processes)
#define add_csp0_process(str) \
    do { \
        csp_id  __process; \
        check0(csp_load_csp0_string(csp, (str), &__process)); \
        csp_id_set_builder_add(&__builder, __process); \
    } while (0)

/* Verify the `initials` of the given CSP₀ process.  `events` should be a
 * (possibly empty) parenthesized list of event names. */
#define check_csp0_initials(csp0, events) \
    do { \
        csp_id  __process; \
        struct csp_id_set_builder  __builder; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
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
        csp_process_deref(csp, __process); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
    } while (0)

/* Verify the `afters` of the given CSP₀ process after performing `initial`.
 * `initial` should be an event name.  `afters` should be a (possibly empty)
 * parenthesized list of CSP₀ processes. */
#define check_csp0_afters(csp0, initial, afters) \
    do { \
        csp_id  __process; \
        csp_id  __initial; \
        struct csp_id_set_builder  __builder; \
        struct csp_id_set  __actual; \
        struct csp_id_set  __expected; \
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
        csp_process_deref(csp, __process); \
        csp_process_set_deref(csp, &__actual); \
        csp_process_set_deref(csp, &__expected); \
        csp_id_set_done(&__actual); \
        csp_id_set_done(&__expected); \
    } while (0)

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* STOP □ STOP */
    check_csp0_initials("STOP □ STOP", ());
    check_csp0_afters("STOP □ STOP", "a", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* (a → STOP) □ (b → STOP ⊓ c → STOP) */
    check_csp0_initials("(a → STOP) □ (b → STOP ⊓ c → STOP)", ("a", "τ"));
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "a", ("STOP"));
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "b", ());
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "τ",
                      ("a → STOP □ b → STOP", "a → STOP □ c → STOP"));
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → STOP) □ (b → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* (a → STOP) □ (b → STOP) */
    check_csp0_initials("(a → STOP) □ (b → STOP)", ("a", "b"));
    check_csp0_afters("(a → STOP) □ (b → STOP)", "a", ("STOP"));
    check_csp0_afters("(a → STOP) □ (b → STOP)", "b", ("STOP"));
    check_csp0_afters("(a → STOP) □ (b → STOP)", "τ", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* □ {a → STOP, b → STOP, c → STOP} */
    check_csp0_initials("□ {a → STOP, b → STOP, c → STOP}", ("a", "b", "c"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "a", ("STOP"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "b", ("STOP"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "c", ("STOP"));
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "τ", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* STOP ⊓ STOP */
    check_csp0_initials("STOP ⊓ STOP", ("τ"));
    check_csp0_afters("STOP ⊓ STOP", "τ", ("STOP"));
    check_csp0_afters("STOP ⊓ STOP", "a", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* (a → STOP) ⊓ (b → STOP) */
    check_csp0_initials("(a → STOP) ⊓ (b → STOP)", ("τ"));
    check_csp0_afters("(a → STOP) ⊓ (b → STOP)", "τ", ("a → STOP", "b → STOP"));
    check_csp0_afters("(a → STOP) ⊓ (b → STOP)", "a", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* ⊓ {a → STOP, b → STOP, c → STOP} */
    check_csp0_initials("⊓ {a → STOP, b → STOP, c → STOP}", ("τ"));
    check_csp0_afters("⊓ {a → STOP, b → STOP, c → STOP}", "τ",
                      ("a → STOP", "b → STOP", "c → STOP"));
    check_csp0_afters("⊓ {a → STOP, b → STOP, c → STOP}", "a", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP */
    check_csp0_initials("a → STOP", ("a"));
    check_csp0_afters("a → STOP", "a", ("STOP"));
    check_csp0_afters("a → STOP", "b", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → b → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → b → STOP */
    check_csp0_initials("a → b → STOP", ("a"));
    check_csp0_afters("a → b → STOP", "a", ("b → STOP"));
    check_csp0_afters("a → b → STOP", "b", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("sequential composition");

TEST_CASE("SKIP ; STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP */
    check_csp0_initials("SKIP ; STOP", ("τ"));
    check_csp0_afters("SKIP ; STOP", "a", ());
    check_csp0_afters("SKIP ; STOP", "b", ());
    check_csp0_afters("SKIP ; STOP", "τ", ("STOP"));
    check_csp0_afters("SKIP ; STOP", "✔", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → SKIP ; STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP */
    check_csp0_initials("a → SKIP ; STOP", ("a"));
    check_csp0_afters("a → SKIP ; STOP", "a", ("SKIP ; STOP"));
    check_csp0_afters("a → SKIP ; STOP", "b", ());
    check_csp0_afters("a → SKIP ; STOP", "τ", ());
    check_csp0_afters("a → SKIP ; STOP", "✔", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → b → STOP □ SKIP) ; STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP */
    check_csp0_initials("(a → b → STOP □ SKIP) ; STOP", ("a", "τ"));
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "a", ("b → STOP ; STOP"));
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "b", ());
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "τ", ("STOP"));
    check_csp0_afters("(a → b → STOP □ SKIP) ; STOP", "✔", ());
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → b → STOP ⊓ SKIP) ; STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP */
    check_csp0_initials("(a → b → STOP ⊓ SKIP) ; STOP", ("τ"));
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "a", ());
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "b", ());
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "τ",
                      ("a → b → STOP ; STOP", "SKIP ; STOP"));
    check_csp0_afters("(a → b → STOP ⊓ SKIP) ; STOP", "✔", ());
    /* Clean up. */
    csp_free(csp);
}
