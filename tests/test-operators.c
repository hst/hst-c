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

TEST_CASE_GROUP("external choice");

TEST_CASE("STOP □ STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* STOP □ STOP */
    check_csp0_initials("STOP □ STOP");
    check_csp0_afters("STOP □ STOP", "a");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → STOP) □ (b → STOP ⊓ c → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* (a → STOP) □ (b → STOP ⊓ c → STOP) */
    check_csp0_initials("(a → STOP) □ (b → STOP ⊓ c → STOP)", "a", "τ");
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "a", "STOP");
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "b");
    check_csp0_afters("(a → STOP) □ (b → STOP ⊓ c → STOP)", "τ",
                      "a → STOP □ b → STOP", "a → STOP □ c → STOP");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → STOP) □ (b → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* (a → STOP) □ (b → STOP) */
    check_csp0_initials("(a → STOP) □ (b → STOP)", "a", "b");
    check_csp0_afters("(a → STOP) □ (b → STOP)", "a", "STOP");
    check_csp0_afters("(a → STOP) □ (b → STOP)", "b", "STOP");
    check_csp0_afters("(a → STOP) □ (b → STOP)", "τ");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("□ {a → STOP, b → STOP, c → STOP}") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* □ {a → STOP, b → STOP, c → STOP} */
    check_csp0_initials("□ {a → STOP, b → STOP, c → STOP}", "a", "b", "c");
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "a", "STOP");
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "b", "STOP");
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "c", "STOP");
    check_csp0_afters("□ {a → STOP, b → STOP, c → STOP}", "τ");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("internal choice");

TEST_CASE("STOP ⊓ STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* STOP ⊓ STOP */
    check_csp0_initials("STOP ⊓ STOP", "τ");
    check_csp0_afters("STOP ⊓ STOP", "τ", "STOP");
    check_csp0_afters("STOP ⊓ STOP", "a");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("(a → STOP) ⊓ (b → STOP)") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* (a → STOP) ⊓ (b → STOP) */
    check_csp0_initials("(a → STOP) ⊓ (b → STOP)", "τ");
    check_csp0_afters("(a → STOP) ⊓ (b → STOP)", "τ", "a → STOP", "b → STOP");
    check_csp0_afters("(a → STOP) ⊓ (b → STOP)", "a");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("⊓ {a → STOP, b → STOP, c → STOP}") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* ⊓ {a → STOP, b → STOP, c → STOP} */
    check_csp0_initials("⊓ {a → STOP, b → STOP, c → STOP}", "τ");
    check_csp0_afters("⊓ {a → STOP, b → STOP, c → STOP}", "τ",
                      "a → STOP", "b → STOP", "c → STOP");
    check_csp0_afters("⊓ {a → STOP, b → STOP, c → STOP}", "a");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → STOP */
    check_csp0_initials("a → STOP", "a");
    check_csp0_afters("a → STOP", "a", "STOP");
    check_csp0_afters("a → STOP", "b");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("a → b → STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* a → b → STOP */
    check_csp0_initials("a → b → STOP", "a");
    check_csp0_afters("a → b → STOP", "a", "b → STOP");
    check_csp0_afters("a → b → STOP", "b");
    /* Clean up. */
    csp_free(csp);
}
