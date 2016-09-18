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
