/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "event.h"

#include "ccan/cppmagic/cppmagic.h"
#include "test-case-harness.h"

static const struct csp_event *
e(const char *name)
{
    return csp_event_get(name);
}

TEST_CASE_GROUP("events sets");

TEST_CASE("can create empty set")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    check(csp_event_set_eq(&set, event_set()));
    csp_event_set_done(&set);
}

TEST_CASE("can add events")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    check(csp_event_set_add(&set, e("a")));
    check(csp_event_set_add(&set, e("b")));
    check(csp_event_set_add(&set, e("c")));
    check(csp_event_set_eq(&set, event_set("a", "b", "c")));
    csp_event_set_done(&set);
}

TEST_CASE("can add duplicate events")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    check(csp_event_set_add(&set, e("a")));
    check(csp_event_set_add(&set, e("b")));
    check(csp_event_set_add(&set, e("c")));
    check(!csp_event_set_add(&set, e("a")));
    check(!csp_event_set_add(&set, e("b")));
    check(!csp_event_set_add(&set, e("c")));
    check(csp_event_set_eq(&set, event_set("a", "b", "c")));
    csp_event_set_done(&set);
}

TEST_CASE("can remove events")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    csp_event_set_add(&set, e("a"));
    csp_event_set_add(&set, e("b"));
    csp_event_set_add(&set, e("c"));
    check(csp_event_set_remove(&set, e("b")));
    check(csp_event_set_eq(&set, event_set("a", "c")));
    csp_event_set_done(&set);
}

TEST_CASE("can remove missing events")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    csp_event_set_add(&set, e("a"));
    csp_event_set_add(&set, e("b"));
    csp_event_set_add(&set, e("c"));
    check(!csp_event_set_remove(&set, e("d")));
    check(csp_event_set_eq(&set, event_set("a", "b", "c")));
    csp_event_set_done(&set);
}

TEST_CASE("can union sets")
{
    struct csp_event_set set1;
    struct csp_event_set set2;
    csp_event_set_init(&set1);
    csp_event_set_add(&set1, e("a"));
    csp_event_set_add(&set1, e("b"));
    csp_event_set_init(&set2);
    csp_event_set_add(&set2, e("c"));
    csp_event_set_union(&set2, &set1);
    check(csp_event_set_eq(&set2, event_set("a", "b", "c")));
    csp_event_set_done(&set1);
    csp_event_set_done(&set2);
}

TEST_CASE("can check subsets")
{
    check(csp_event_set_subseteq(event_set(), event_set()));
    check(csp_event_set_subseteq(event_set(), event_set("a")));
    check(!csp_event_set_subseteq(event_set("c"), event_set()));
    check(!csp_event_set_subseteq(event_set("c"), event_set("b")));
    check(csp_event_set_subseteq(event_set("c"), event_set("b", "c")));
    check(csp_event_set_subseteq(event_set("c"), event_set("b", "c", "d")));
    check(csp_event_set_subseteq(event_set("c"), event_set("c")));
    check(csp_event_set_subseteq(event_set("c"), event_set("c", "d")));
    check(!csp_event_set_subseteq(event_set("c"), event_set("d")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set()));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("a")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("a", "b")));
    check(csp_event_set_subseteq(event_set("b", "c"),
                                 event_set("a", "b", "c")));
    check(csp_event_set_subseteq(event_set("b", "c"),
                                 event_set("a", "b", "c", "d")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("a", "c")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("b")));
    check(csp_event_set_subseteq(event_set("b", "c"), event_set("b", "c")));
    check(csp_event_set_subseteq(event_set("b", "c"),
                                 event_set("b", "c", "d")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("b", "d")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("c")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("c", "d")));
    check(!csp_event_set_subseteq(event_set("b", "c"), event_set("d")));
}
