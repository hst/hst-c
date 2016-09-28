/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "hst.h"
#include "test-case-harness.h"

TEST_CASE_GROUP("prefix");

TEST_CASE("a → STOP") {
    struct csp  *csp;
    csp_id  a;
    csp_id  b;
    csp_id  root;
    struct csp_id_set  set;
    /* Create the CSP environment. */
    csp_id_set_init(&set);
    check_alloc(csp, csp_new());
    /* root = a → STOP */
    a = csp_get_event_id(csp, "a");
    b = csp_get_event_id(csp, "b");
    root = csp_prefix(csp, a, csp_process_ref(csp, csp->stop));
    /* initials(root) == {a} */
    csp_process_get_initials(csp, root, &set);
    check_set_elements_msg("initials(a → STOP) == {a}", set, a);
    /* afters(root, a) == {STOP} */
    csp_process_get_afters(csp, root, a, &set);
    check_set_elements_msg("afters(a → STOP, a) == {STOP}", set, csp->stop);
    /* afters(root, b) == {} */
    csp_process_get_afters(csp, root, b, &set);
    check_set_empty_msg("afters(a → STOP, b) == {}", set);
    /* Clean up. */
    csp_id_set_done(&set);
    csp_process_deref(csp, root);
    csp_free(csp);
}

TEST_CASE("a → b → STOP") {
    struct csp  *csp;
    csp_id  a;
    csp_id  b;
    csp_id  root;
    csp_id  p1;
    struct csp_id_set  set;
    /* Create the CSP environment. */
    csp_id_set_init(&set);
    check_alloc(csp, csp_new());
    /* root = a → b → STOP */
    a = csp_get_event_id(csp, "a");
    b = csp_get_event_id(csp, "b");
    p1 = csp_prefix(csp, b, csp_process_ref(csp, csp->stop));
    root = csp_prefix(csp, a, csp_process_ref(csp, p1));
    /* initials(root) == {a} */
    csp_process_get_initials(csp, root, &set);
    check_set_elements_msg("initials(a → b → STOP) == {a}", set, a);
    /* afters(root, a) == {p1} */
    csp_process_get_afters(csp, root, a, &set);
    check_set_elements_msg("afters(a → b → STOP, a) = {b → STOP}", set, p1);
    /* afters(root, b) == {} */
    csp_process_get_afters(csp, root, b, &set);
    check_set_empty_msg("afters(a → b → STOP, b) = {}", set);
    /* initials(p1) == {b} */
    csp_process_get_initials(csp, p1, &set);
    check_set_elements_msg("initials(b → STOP) == {b}", set, b);
    /* afters(p1, a) == {} */
    csp_process_get_afters(csp, p1, a, &set);
    check_set_empty_msg("afters(b → STOP, a) = {}", set);
    /* afters(p1, b) == {STOP} */
    csp_process_get_afters(csp, p1, b, &set);
    check_set_elements_msg("afters(b → STOP, b) = {STOP}", set, csp->stop);
    /* Clean up. */
    csp_id_set_done(&set);
    csp_process_deref(csp, root);
    csp_process_deref(csp, p1);
    csp_free(csp);
}
