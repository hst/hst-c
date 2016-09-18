/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <string.h>

#include "hst.h"
#include "test-case-harness.h"

TEST_CASE_GROUP("environments");

#define check_size(set, expected) \
    check_with_msg((set).count == (expected), \
            "Expected set to have size %zu, got %zu", \
            (size_t) (expected), (set).count)

#define check_elements(set, ...) \
    do { \
        csp_id  __expected[] = { __VA_ARGS__ }; \
        size_t  __count = sizeof(__expected) / sizeof(__expected[0]); \
        size_t  __i; \
        for (__i = 0; __i < __count; __i++) { \
            check_with_msg((set).ids[__i] == __expected[__i], \
                    "Expected set[%zu] to be %lu, got %lu", \
                    __i, __expected[__i], (set).ids[__i]); \
        } \
    } while (0)

#define check_streq(actual, expected) \
    check_with_msg(strcmp((actual), (expected)) == 0, \
            "Expected \"%s\", got \"%s\"", (expected), (actual))

TEST_CASE("predefined events exist") {
    static const char* const TAU = "τ";
    static const char* const TICK = "✔";
    struct csp  *csp;
    check_alloc(csp, csp_new());
    check_streq(csp_get_event_name(csp, csp_tau(csp)), TAU);
    check_streq(csp_get_event_name(csp, csp_tick(csp)), TICK);
    csp_free(csp);
}

TEST_CASE("can create events") {
    struct csp  *csp;
    check_alloc(csp, csp_new());
    check_with_msg(csp_get_event_name(csp, 0) == NULL,
                   "Should get a name for undefined event");
    check_streq(csp_get_event_name(csp, csp_get_event_id(csp, "a")), "a");
    check_streq(csp_get_event_name(csp, csp_get_event_id(csp, "b")), "b");
    csp_free(csp);
}

TEST_CASE("predefined STOP process exists") {
    struct csp  *csp;
    csp_id  tau;
    csp_id  tick;
    csp_id  stop;
    struct csp_id_set  set;
    /* Create the CSP environment. */
    csp_id_set_init(&set);
    check_alloc(csp, csp_new());
    tau = csp_tau(csp);
    tick = csp_tick(csp);
    stop = csp_stop(csp);
    /* Verify the initials set of the STOP process. */
    csp_process_get_initials(csp, stop, &set);
    check_size(set, 0);
    /* Verify the afters of τ. */
    csp_process_get_afters(csp, stop, tau, &set);
    check_size(set, 0);
    /* Verify the afters of ✔. */
    csp_process_get_afters(csp, stop, tick, &set);
    check_size(set, 0);
    /* Clean up. */
    csp_id_set_done(&set);
    csp_process_deref(csp, stop);
    csp_free(csp);
}

TEST_CASE("predefined SKIP process exists") {
    struct csp  *csp;
    csp_id  tau;
    csp_id  tick;
    csp_id  stop;
    csp_id  skip;
    struct csp_id_set  set;
    /* Create the CSP environment. */
    csp_id_set_init(&set);
    check_alloc(csp, csp_new());
    tau = csp_tau(csp);
    tick = csp_tick(csp);
    stop = csp_stop(csp);
    skip = csp_skip(csp);
    /* Verify the initials set of the SKIP process. */
    csp_process_get_initials(csp, skip, &set);
    check_size(set, 1);
    check_elements(set, tick);
    /* Verify the afters of τ. */
    csp_process_get_afters(csp, skip, tau, &set);
    check_size(set, 0);
    /* Verify the afters of ✔. */
    csp_process_get_afters(csp, skip, tick, &set);
    check_size(set, 1);
    check_elements(set, stop);
    /* Clean up. */
    csp_id_set_done(&set);
    csp_process_deref(csp, stop);
    csp_process_deref(csp, skip);
    csp_free(csp);
}
