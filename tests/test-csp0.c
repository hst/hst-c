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

/* Don't check the semantics of any of the operators here; this file just checks
 * that the CSP₀ parser produces the same processes that you'd get constructing
 * things by hand.  Look in test-operators.c for test cases that verify that
 * each operator behaves as we expect it to. */

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

TEST_CASE("can parse external choice") {
    struct csp  *csp;
    csp_id  a;
    csp_id  p1;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    root = csp_external_choice(csp, p1, csp_process_ref(csp, csp->skip));
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a->STOP[]SKIP", root);
    check_csp0_eq(" a->STOP[]SKIP", root);
    check_csp0_eq(" a ->STOP[]SKIP", root);
    check_csp0_eq(" a -> STOP[]SKIP", root);
    check_csp0_eq(" a -> STOP []SKIP", root);
    check_csp0_eq(" a -> STOP [] SKIP", root);
    check_csp0_eq(" a -> STOP [] SKIP ", root);
    check_csp0_eq("a→STOP□SKIP", root);
    check_csp0_eq(" a→STOP□SKIP", root);
    check_csp0_eq(" a →STOP□SKIP", root);
    check_csp0_eq(" a → STOP□SKIP", root);
    check_csp0_eq(" a → STOP □SKIP", root);
    check_csp0_eq(" a → STOP □ SKIP", root);
    check_csp0_eq(" a → STOP □ SKIP ", root);
    /* Fail to parse a bunch of invalid statements. */
    /* a is undefined */
    check_csp0_invalid("a □ STOP");
    check_csp0_invalid("STOP □ a");
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("external choice is right-associative") {
    struct csp  *csp;
    csp_id  a;
    csp_id  b;
    csp_id  p1;
    csp_id  p2;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    b = csp_get_event_id(csp, "b");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    p2 = csp_prefix(csp, b, csp_process_ref(csp, csp->stop));
    root = csp_external_choice(csp, p1, p2);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a -> STOP [] b -> STOP", root);
    check_csp0_eq("a → STOP □ b → STOP", root);
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("can parse internal choice") {
    struct csp  *csp;
    csp_id  a;
    csp_id  p1;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    root = csp_internal_choice(csp, p1, csp_process_ref(csp, csp->skip));
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a->STOP|~|SKIP", root);
    check_csp0_eq(" a->STOP|~|SKIP", root);
    check_csp0_eq(" a ->STOP|~|SKIP", root);
    check_csp0_eq(" a -> STOP|~|SKIP", root);
    check_csp0_eq(" a -> STOP |~|SKIP", root);
    check_csp0_eq(" a -> STOP |~| SKIP", root);
    check_csp0_eq(" a -> STOP |~| SKIP ", root);
    check_csp0_eq("a→STOP⊓SKIP", root);
    check_csp0_eq(" a→STOP⊓SKIP", root);
    check_csp0_eq(" a →STOP⊓SKIP", root);
    check_csp0_eq(" a → STOP⊓SKIP", root);
    check_csp0_eq(" a → STOP ⊓SKIP", root);
    check_csp0_eq(" a → STOP ⊓ SKIP", root);
    check_csp0_eq(" a → STOP ⊓ SKIP ", root);
    /* Fail to parse a bunch of invalid statements. */
    /* a is undefined */
    check_csp0_invalid("a ⊓ STOP");
    check_csp0_invalid("STOP ⊓ a");
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("internal choice is right-associative") {
    struct csp  *csp;
    csp_id  a;
    csp_id  b;
    csp_id  p1;
    csp_id  p2;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    b = csp_get_event_id(csp, "b");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    p2 = csp_prefix(csp, b, csp_process_ref(csp, csp->stop));
    root = csp_internal_choice(csp, p1, p2);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a -> STOP |~| b -> STOP", root);
    check_csp0_eq("a → STOP ⊓ b → STOP", root);
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

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

TEST_CASE("can parse replicated external choice") {
    struct csp  *csp;
    csp_id  a;
    csp_id  p1;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    root = csp_external_choice(csp, p1, csp_process_ref(csp, csp->skip));
    fprintf(stderr, "STOP = 0x%08lx\n", csp->stop);
    fprintf(stderr, "SKIP = 0x%08lx\n", csp->skip);
    fprintf(stderr, "p1   = 0x%08lx\n", p1);
    fprintf(stderr, "root = 0x%08lx\n", root);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("[]{a->STOP,SKIP}", root);
    check_csp0_eq(" []{a->STOP,SKIP}", root);
    check_csp0_eq(" [] {a->STOP,SKIP}", root);
    check_csp0_eq(" [] { a->STOP,SKIP}", root);
    check_csp0_eq(" [] { a ->STOP,SKIP}", root);
    check_csp0_eq(" [] { a -> STOP,SKIP}", root);
    check_csp0_eq(" [] { a -> STOP ,SKIP}", root);
    check_csp0_eq(" [] { a -> STOP , SKIP}", root);
    check_csp0_eq(" [] { a -> STOP , SKIP }", root);
    check_csp0_eq(" [] { a -> STOP , SKIP } ", root);
    check_csp0_eq("□{a→STOP,SKIP}", root);
    check_csp0_eq(" □{a→STOP,SKIP}", root);
    check_csp0_eq(" □ {a→STOP,SKIP}", root);
    check_csp0_eq(" □ { a→STOP,SKIP}", root);
    check_csp0_eq(" □ { a →STOP,SKIP}", root);
    check_csp0_eq(" □ { a → STOP,SKIP}", root);
    check_csp0_eq(" □ { a → STOP ,SKIP}", root);
    check_csp0_eq(" □ { a → STOP , SKIP}", root);
    check_csp0_eq(" □ { a → STOP , SKIP }", root);
    /* missing `{` */
    check_csp0_invalid("□");
    /* missing process after `{` */
    check_csp0_invalid("□ {");
    /* missing `}` */
    check_csp0_invalid("□ { STOP");
    /* missing process after `,` */
    check_csp0_invalid("□ { STOP,");
    check_csp0_invalid("□ { STOP, }");
    /* a is undefined */
    check_csp0_invalid("□ { a, STOP }");
    check_csp0_invalid("□ { STOP, a }");
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("can parse replicated internal choice") {
    struct csp  *csp;
    csp_id  a;
    csp_id  p1;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    a = csp_get_event_id(csp, "a");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    root = csp_internal_choice(csp, p1, csp_process_ref(csp, csp->skip));
    fprintf(stderr, "STOP = 0x%08lx\n", csp->stop);
    fprintf(stderr, "SKIP = 0x%08lx\n", csp->skip);
    fprintf(stderr, "p1   = 0x%08lx\n", p1);
    fprintf(stderr, "root = 0x%08lx\n", root);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("|~|{a->STOP,SKIP}", root);
    check_csp0_eq(" |~|{a->STOP,SKIP}", root);
    check_csp0_eq(" |~| {a->STOP,SKIP}", root);
    check_csp0_eq(" |~| { a->STOP,SKIP}", root);
    check_csp0_eq(" |~| { a ->STOP,SKIP}", root);
    check_csp0_eq(" |~| { a -> STOP,SKIP}", root);
    check_csp0_eq(" |~| { a -> STOP ,SKIP}", root);
    check_csp0_eq(" |~| { a -> STOP , SKIP}", root);
    check_csp0_eq(" |~| { a -> STOP , SKIP }", root);
    check_csp0_eq(" |~| { a -> STOP , SKIP } ", root);
    check_csp0_eq("⊓{a→STOP,SKIP}", root);
    check_csp0_eq(" ⊓{a→STOP,SKIP}", root);
    check_csp0_eq(" ⊓ {a→STOP,SKIP}", root);
    check_csp0_eq(" ⊓ { a→STOP,SKIP}", root);
    check_csp0_eq(" ⊓ { a →STOP,SKIP}", root);
    check_csp0_eq(" ⊓ { a → STOP,SKIP}", root);
    check_csp0_eq(" ⊓ { a → STOP ,SKIP}", root);
    check_csp0_eq(" ⊓ { a → STOP , SKIP}", root);
    check_csp0_eq(" ⊓ { a → STOP , SKIP }", root);
    /* missing `{` */
    check_csp0_invalid("⊓");
    /* missing process after `{` */
    check_csp0_invalid("⊓ {");
    /* missing `}` */
    check_csp0_invalid("⊓ { STOP");
    /* missing process after `,` */
    check_csp0_invalid("⊓ { STOP,");
    check_csp0_invalid("⊓ { STOP, }");
    /* a is undefined */
    check_csp0_invalid("⊓ { a, STOP }");
    check_csp0_invalid("⊓ { STOP, a }");
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("verify precedence order") {
    struct csp  *csp;
    csp_id  a;
    csp_id  b;
    csp_id  c;
    csp_id  p1;
    csp_id  p2;
    csp_id  p3;
    csp_id  p4;
    csp_id  root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Expected result is
     * (a → STOP □ b → STOP) ⊓ (c → STOP)
     */
    a = csp_get_event_id(csp, "a");
    b = csp_get_event_id(csp, "b");
    c = csp_get_event_id(csp, "c");
    p1 = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    p2 = csp_prefix(csp, b, csp_process_ref(csp, csp->stop));
    p3 = csp_prefix(csp, c, csp_process_ref(csp, csp->stop));
    p4 = csp_external_choice(csp, p1, p2);
    root = csp_internal_choice(csp, p4, p3);
    /* Verify the precedence order of operations. */
    check_csp0_eq("a → STOP □ b → STOP ⊓ c → STOP", root);
    /* Clean up. */
    csp_process_deref(csp, root);
    csp_free(csp);
}
