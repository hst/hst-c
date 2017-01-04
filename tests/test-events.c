/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "event.h"

#include "test-case-harness.h"
#include "test-cases.h"

TEST_CASE_GROUP("events");

TEST_CASE("predefined events exist")
{
    check_streq(csp_event_name(csp_tau()), "τ");
    check_streq(csp_event_name(csp_tick()), "✔");
}

TEST_CASE("can create events")
{
    check_streq(csp_event_name(csp_event_get("a")), "a");
    check_streq(csp_event_name(csp_event_get_sized("a", 1)), "a");
    check_streq(csp_event_name(csp_event_get("b")), "b");
    check_streq(csp_event_name(csp_event_get_sized("b", 1)), "b");
}
