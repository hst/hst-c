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
