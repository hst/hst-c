/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "csp0.h"

#include <string.h>

#include "environment.h"
#include "event.h"
#include "operators.h"
#include "process.h"
#include "test-case-harness.h"
#include "test-cases.h"

/* Don't check the semantics of any of the operators here; this file just checks
 * that the CSP₀ parser produces the same processes that you'd get constructing
 * things by hand.  Look in test-operators.c for test cases that verify that
 * each operator behaves as we expect it to. */

#define check_csp0_eq(str, expected)                                \
    do {                                                            \
        struct csp_process *__actual;                               \
        check_nonnull(__actual = csp_load_csp0_string(csp, (str))); \
        check(__actual == (expected));                              \
    } while (0)

#define check_csp0_valid(str)                            \
    do {                                                 \
        check_nonnull(csp_load_csp0_string(csp, (str))); \
    } while (0)

#define check_csp0_invalid(str)                          \
    do {                                                 \
        check(csp_load_csp0_string(csp, (str)) == NULL); \
    } while (0)

TEST_CASE_GROUP("CSP₀ syntax");

TEST_CASE("can parse identifiers")
{
    struct csp *csp;
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

TEST_CASE("can parse debug recursion identifiers")
{
    struct csp *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Parse a bunch of valid identifiers. */
    check_csp0_valid("let X = a → STOP within X@0");
    check_csp0_valid("let X = let Y = a → STOP within X@1 within STOP");
    /* Fail to parse a bunch of invalid identifiers. */
    check_csp0_invalid("let X = a → STOP within X@");
    check_csp0_invalid("let X = a → STOP within X@X");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE_GROUP("CSP₀ primitives");

TEST_CASE("parse: STOP")
{
    struct csp *csp;
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

TEST_CASE("parse: SKIP")
{
    struct csp *csp;
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

TEST_CASE("parse: a → STOP □ SKIP")
{
    struct csp *csp;
    struct csp_process *p0;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p0 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    root = csp_external_choice(csp, p0, csp->skip);
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
    csp_free(csp);
}

TEST_CASE("associativity: a → STOP □ b → STOP □ c → STOP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *p2;
    struct csp_process *p3;
    struct csp_process *p4;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    p2 = csp_prefix(csp, csp_event_get("b"), csp->stop);
    p3 = csp_prefix(csp, csp_event_get("c"), csp->stop);
    p4 = csp_external_choice(csp, p2, p3);
    root = csp_external_choice(csp, p1, p4);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a -> STOP [] b -> STOP [] c -> STOP", root);
    check_csp0_eq("a → STOP □ b → STOP □ c → STOP", root);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("parse: a → STOP ⊓ SKIP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    root = csp_internal_choice(csp, p1, csp->skip);
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
    csp_free(csp);
}

TEST_CASE("associativity: a → STOP ⊓ b → STOP ⊓ c → STOP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *p2;
    struct csp_process *p3;
    struct csp_process *p4;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    p2 = csp_prefix(csp, csp_event_get("b"), csp->stop);
    p3 = csp_prefix(csp, csp_event_get("c"), csp->stop);
    p4 = csp_internal_choice(csp, p2, p3);
    root = csp_internal_choice(csp, p1, p4);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a -> STOP |~| b -> STOP |~| c -> STOP", root);
    check_csp0_eq("a → STOP ⊓ b → STOP ⊓ c → STOP", root);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("parse: (STOP)")
{
    struct csp *csp;
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

TEST_CASE("parse: a → STOP")
{
    struct csp *csp;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    root = csp_prefix(csp, csp_event_get("a"), csp->stop);
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
    csp_free(csp);
}

TEST_CASE("associativity: a → b → STOP")
{
    struct csp *csp;
    struct csp_process *p;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p = csp_prefix(csp, csp_event_get("b"), csp->stop);
    root = csp_prefix(csp, csp_event_get("a"), p);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a -> b -> STOP", root);
    check_csp0_eq("a → b → STOP", root);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("parse: let X = a → STOP within X")
{
    struct csp *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_valid("let X=a→STOP within X");
    check_csp0_valid(" let X=a→STOP within X");
    check_csp0_valid(" let X =a→STOP within X");
    check_csp0_valid(" let X = a→STOP within X");
    check_csp0_valid(" let X = a →STOP within X");
    check_csp0_valid(" let X = a → STOP within X");
    check_csp0_valid(" let X = a → STOP within X ");
    /* Fail to parse a bunch of invalid statements. */
    /* missing process definition */
    check_csp0_invalid("let within X");
    /* undefined process */
    check_csp0_invalid("let X = a → Y within X");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("parse: let X = a → Y Y = b → X within X")
{
    struct csp *csp;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_valid("let X=a→Y Y=b→X within X");
    check_csp0_valid(" let X=a→Y Y=b→X within X");
    check_csp0_valid(" let X =a→Y Y=b→X within X");
    check_csp0_valid(" let X = a→Y Y=b→X within X");
    check_csp0_valid(" let X = a →Y Y=b→X within X");
    check_csp0_valid(" let X = a → Y Y=b→X within X");
    check_csp0_valid(" let X = a → Y Y =b→X within X");
    check_csp0_valid(" let X = a → Y Y = b→X within X");
    check_csp0_valid(" let X = a → Y Y = b →X within X");
    check_csp0_valid(" let X = a → Y Y = b → X within X");
    check_csp0_valid(" let X = a → Y Y = b → X within X ");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("parse: ⫴ {a → STOP, SKIP}")
{
    struct csp *csp;
    struct csp_process_set ps;
    struct csp_process *p1;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    csp_process_set_init(&ps);
    csp_process_set_add(&ps, p1);
    csp_process_set_add(&ps, csp->skip);
    root = csp_interleave(csp, &ps);
    csp_process_set_done(&ps);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("|||{a->STOP,SKIP}", root);
    check_csp0_eq(" |||{a->STOP,SKIP}", root);
    check_csp0_eq(" ||| {a->STOP,SKIP}", root);
    check_csp0_eq(" ||| { a->STOP,SKIP}", root);
    check_csp0_eq(" ||| { a ->STOP,SKIP}", root);
    check_csp0_eq(" ||| { a -> STOP,SKIP}", root);
    check_csp0_eq(" ||| { a -> STOP ,SKIP}", root);
    check_csp0_eq(" ||| { a -> STOP , SKIP}", root);
    check_csp0_eq(" ||| { a -> STOP , SKIP }", root);
    check_csp0_eq(" ||| { a -> STOP , SKIP } ", root);
    check_csp0_eq("⫴{a→STOP,SKIP}", root);
    check_csp0_eq(" ⫴{a→STOP,SKIP}", root);
    check_csp0_eq(" ⫴ {a→STOP,SKIP}", root);
    check_csp0_eq(" ⫴ { a→STOP,SKIP}", root);
    check_csp0_eq(" ⫴ { a →STOP,SKIP}", root);
    check_csp0_eq(" ⫴ { a → STOP,SKIP}", root);
    check_csp0_eq(" ⫴ { a → STOP ,SKIP}", root);
    check_csp0_eq(" ⫴ { a → STOP , SKIP}", root);
    check_csp0_eq(" ⫴ { a → STOP , SKIP }", root);
    /* missing `{` */
    check_csp0_invalid("⫴");
    /* missing process after `{` */
    check_csp0_invalid("⫴ {");
    /* missing `}` */
    check_csp0_invalid("⫴ { STOP");
    /* missing process after `,` */
    check_csp0_invalid("⫴ { STOP,");
    check_csp0_invalid("⫴ { STOP, }");
    /* a is undefined */
    check_csp0_invalid("⫴ { a, STOP }");
    check_csp0_invalid("⫴ { STOP, a }");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("parse: □ {a → STOP, SKIP}")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    root = csp_external_choice(csp, p1, csp->skip);
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
    csp_free(csp);
}

TEST_CASE("parse: ⊓ {a → STOP, SKIP}")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    root = csp_internal_choice(csp, p1, csp->skip);
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
    csp_free(csp);
}

TEST_CASE("parse: a → SKIP ; STOP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->skip);
    root = csp_sequential_composition(csp, p1, csp->stop);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a→SKIP;STOP", root);
    check_csp0_eq(" a→SKIP;STOP", root);
    check_csp0_eq(" a →SKIP;STOP", root);
    check_csp0_eq(" a → SKIP;STOP", root);
    check_csp0_eq(" a → SKIP ;STOP", root);
    check_csp0_eq(" a → SKIP ; STOP", root);
    check_csp0_eq(" a → SKIP ; STOP ", root);
    /* Fail to parse a bunch of invalid statements. */
    /* a is undefined */
    check_csp0_invalid("a ; STOP");
    check_csp0_invalid("STOP ; a");
    /* Missing process after ; */
    check_csp0_invalid("SKIP;");
    check_csp0_invalid("SKIP ;");
    check_csp0_invalid("SKIP ; ");
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("associativity: a → SKIP ; b → SKIP ; c → SKIP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *p2;
    struct csp_process *p3;
    struct csp_process *p4;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    p1 = csp_prefix(csp, csp_event_get("a"), csp->skip);
    p2 = csp_prefix(csp, csp_event_get("b"), csp->skip);
    p3 = csp_prefix(csp, csp_event_get("c"), csp->skip);
    p4 = csp_sequential_composition(csp, p2, p3);
    root = csp_sequential_composition(csp, p1, p4);
    /* Verify that we can parse the process, with and without whitespace. */
    check_csp0_eq("a → SKIP ; b → SKIP ; c → SKIP", root);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("precedence: a → STOP □ b → STOP ⊓ c → STOP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *p2;
    struct csp_process *p3;
    struct csp_process *p4;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Expected result is
     * (a → STOP □ b → STOP) ⊓ (c → STOP)
     */
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    p2 = csp_prefix(csp, csp_event_get("b"), csp->stop);
    p3 = csp_prefix(csp, csp_event_get("c"), csp->stop);
    p4 = csp_external_choice(csp, p1, p2);
    root = csp_internal_choice(csp, p4, p3);
    /* Verify the precedence order of operations. */
    check_csp0_eq("a → STOP □ b → STOP ⊓ c → STOP", root);
    /* Clean up. */
    csp_free(csp);
}

TEST_CASE("precedence: a → STOP □ b → SKIP ; c → STOP")
{
    struct csp *csp;
    struct csp_process *p1;
    struct csp_process *p2;
    struct csp_process *p3;
    struct csp_process *p4;
    struct csp_process *root;
    /* Create the CSP environment. */
    check_alloc(csp, csp_new());
    /* Expected result is
     * a → STOP □ (b → SKIP ; c → STOP)
     */
    p1 = csp_prefix(csp, csp_event_get("a"), csp->stop);
    p2 = csp_prefix(csp, csp_event_get("b"), csp->skip);
    p3 = csp_prefix(csp, csp_event_get("c"), csp->stop);
    p4 = csp_sequential_composition(csp, p2, p3);
    root = csp_external_choice(csp, p1, p4);
    /* Verify the precedence order of operations. */
    check_csp0_eq("a → STOP □ b → SKIP ; c → STOP", root);
    /* Clean up. */
    csp_free(csp);
}
