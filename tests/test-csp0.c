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

TEST_CASE_GROUP("CSP₀ syntax");

TEST_CASE("can parse identifiers") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Parse a bunch of valid identifiers. */
    check_csp0_valid("r → STOP");
    check_csp0_valid("r0 → STOP");
    check_csp0_valid("r0r → STOP");
    check_csp0_valid("root → STOP");
    check_csp0_valid("root.root → STOP");
    check_csp0_valid("root_root → STOP");
    check_csp0_valid("_ → STOP");
    check_csp0_valid("_r → STOP");
    check_csp0_valid("_r0 → STOP");
    check_csp0_valid("_r0r → STOP");
    check_csp0_valid("_root → STOP");
    check_csp0_valid("_root.root → STOP");
    check_csp0_valid("_root_root → STOP");
    check_csp0_valid("$r → STOP");
    check_csp0_valid("$r0 → STOP");
    check_csp0_valid("$r0r → STOP");
    check_csp0_valid("$root → STOP");
    check_csp0_valid("$root.root → STOP");
    check_csp0_valid("$root_root → STOP");
    /* Fail to parse a bunch of invalid identifiers. */
    check_csp0_invalid("0 → STOP");
    check_csp0_invalid("$ → STOP");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("CSP₀ primitives");

TEST_CASE("can parse STOP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("STOP", csp->stop);
    check_csp0_eq(" STOP", csp->stop);
    check_csp0_eq("STOP ", csp->stop);
    check_csp0_eq(" STOP ", csp->stop);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("can parse SKIP") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("SKIP", csp->skip);
    check_csp0_eq(" SKIP", csp->skip);
    check_csp0_eq("SKIP ", csp->skip);
    check_csp0_eq(" SKIP ", csp->skip);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("CSP₀ operators");

TEST_CASE("can parse parentheses") {
    struct csp  *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("(STOP)", csp->stop);
    check_csp0_eq(" (STOP)", csp->stop);
    check_csp0_eq(" ( STOP)", csp->stop);
    check_csp0_eq(" ( STOP )", csp->stop);
    check_csp0_eq(" ( STOP ) ", csp->stop);
    check_csp0_eq("((STOP))", csp->stop);
    check_csp0_eq("(((STOP)))", csp->stop);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("can parse prefix") {
    struct csp  *csp;
    csp_id  a;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    root = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a->STOP", root);
    check_csp0_eq(" a->STOP", root);
    check_csp0_eq(" a ->STOP", root);
    check_csp0_eq(" a -> STOP", root);
    check_csp0_eq(" a -> STOP ", root);
    check_csp0_eq("a→STOP", root);
    check_csp0_eq(" a→STOP", root);
    check_csp0_eq(" a →STOP", root);
    check_csp0_eq(" a → STOP", root);
    check_csp0_eq(" a → STOP ", root);
    /* Fail to parse a bunch of invalid statements. */
    /* STOP isn't an event */
    check_csp0_invalid("STOP → STOP");
    /* undefined isn't defined */
    check_csp0_invalid("a → undefined");
    /* b isn't a process (reported as "b is undefined") */
    check_csp0_invalid("(a → b) → STOP");
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("prefix is right-associative") {
    struct csp  *csp;
    csp_id  a;
    csp_id  b;
    csp_id  p;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    b = csp_get_event_id(csp, "b");
    p = csp_prefix(csp, b, csp_process_ref(csp, csp->stop));
    root = csp_prefix(csp, a, p);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a -> b -> STOP", root);
    check_csp0_eq("a → b → STOP", root);
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}
