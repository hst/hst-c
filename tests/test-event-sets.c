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

static void
csp_event_set_free_(void *vset)
{
    struct csp_event_set *set = vset;
    csp_event_set_done(set);
    free(set);
}

/* Creates a new empty set.  The set will be automatically freed for you at the
 * end of the test case. */
static struct csp_event_set *
empty_event_set(void)
{
    struct csp_event_set *set = malloc(sizeof(struct csp_event_set));
    assert(set != NULL);
    csp_event_set_init(set);
    test_case_cleanup_register(csp_event_set_free_, set);
    return set;
}

/* Creates a new set containing the given events.  The set will be automatically
 * freed for you at the end of the test case. */
#define event_set(...)                                 \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
    (event_set_(LENGTH(__VA_ARGS__), __VA_ARGS__))(empty_event_set())

static struct csp_event_set *
event_set_(size_t count, ...)
{
    size_t i;
    va_list args;
    struct csp_event_set *set = malloc(sizeof(struct csp_event_set));
    assert(set != NULL);
    csp_event_set_init(set);
    test_case_cleanup_register(csp_event_set_free_, set);
    va_start(args, count);
    for (i = 0; i < count; i++) {
        const char *name = va_arg(args, const char *);
        csp_event_set_add(set, e(name));
    }
    va_end(args);
    return set;
}

TEST_CASE_GROUP("events sets");

TEST_CASE("can create empty set")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    check(csp_event_set_eq(&set, event_set()));
    csp_event_set_done(&set);
}

TEST_CASE("can add individual events")
{
    struct csp_event_set set;
    csp_event_set_init(&set);
    check(csp_event_set_add(&set, e("a")));
    check(csp_event_set_add(&set, e("b")));
    check(csp_event_set_add(&set, e("c")));
    check(csp_event_set_eq(&set, event_set("a", "b", "c")));
    csp_event_set_done(&set);
}

TEST_CASE("can add duplicate individual events")
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

TEST_CASE("can remove individual events")
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

TEST_CASE("can remove missing individual events")
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
