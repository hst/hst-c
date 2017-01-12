/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "process.h"

#include "test-case-harness.h"

static struct csp_process p0;
static struct csp_process p1;
static struct csp_process p2;
static struct csp_process p5;
static struct csp_process p6;

static void
csp_process_set_free_(void *vset)
{
    struct csp_process_set *set = vset;
    csp_process_set_done(set);
    free(set);
}

/* Creates a new empty set.  The set will be automatically freed for you at the
 * end of the test case. */
static struct csp_process_set *
empty_process_set(void)
{
    struct csp_process_set *set = malloc(sizeof(struct csp_process_set));
    assert(set != NULL);
    csp_process_set_init(set);
    test_case_cleanup_register(csp_process_set_free_, set);
    return set;
}

/* Creates a new set containing the given processes.  The set will be
 * automatically freed for you at the end of the test case. */
#define process_set(...)                                 \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
    (process_set_(LENGTH(__VA_ARGS__), __VA_ARGS__))(empty_process_set())

static struct csp_process_set *
process_set_(size_t count, ...)
{
    size_t i;
    va_list args;
    struct csp_process_set *set = malloc(sizeof(struct csp_process_set));
    assert(set != NULL);
    csp_process_set_init(set);
    test_case_cleanup_register(csp_process_set_free_, set);
    va_start(args, count);
    for (i = 0; i < count; i++) {
        struct csp_process *process = va_arg(args, struct csp_process *);
        csp_process_set_add(set, process);
    }
    va_end(args);
    return set;
}

TEST_CASE_GROUP("process sets");

TEST_CASE("can create empty set")
{
    struct csp_process_set set;
    csp_process_set_init(&set);
    check(csp_process_set_eq(&set, process_set()));
    csp_process_set_done(&set);
}

TEST_CASE("can add ids")
{
    struct csp_process_set set;
    csp_process_set_init(&set);
    check(csp_process_set_add(&set, &p0));
    check(csp_process_set_add(&set, &p5));
    check(csp_process_set_add(&set, &p1));
    check(csp_process_set_eq(&set, process_set(&p0, &p1, &p5)));
    csp_process_set_done(&set);
}

TEST_CASE("can add duplicate ids")
{
    struct csp_process_set set;
    csp_process_set_init(&set);
    check(csp_process_set_add(&set, &p0));
    check(csp_process_set_add(&set, &p5));
    check(csp_process_set_add(&set, &p1));
    check(!csp_process_set_add(&set, &p5));
    check(!csp_process_set_add(&set, &p0));
    check(!csp_process_set_add(&set, &p0));
    check(csp_process_set_eq(&set, process_set(&p0, &p1, &p5)));
    csp_process_set_done(&set);
}

TEST_CASE("can remove ids")
{
    struct csp_process_set set;
    csp_process_set_init(&set);
    csp_process_set_add(&set, &p0);
    csp_process_set_add(&set, &p5);
    csp_process_set_add(&set, &p1);
    check(csp_process_set_remove(&set, &p5));
    check(!csp_process_set_remove(&set, &p6));
    check(csp_process_set_eq(&set, process_set(&p0, &p1)));
    csp_process_set_done(&set);
}

TEST_CASE("can remove missing ids")
{
    struct csp_process_set set;
    csp_process_set_init(&set);
    csp_process_set_add(&set, &p0);
    csp_process_set_add(&set, &p5);
    csp_process_set_add(&set, &p1);
    csp_process_set_remove(&set, &p5);
    csp_process_set_remove(&set, &p2);
    check(csp_process_set_eq(&set, process_set(&p0, &p1)));
    csp_process_set_done(&set);
}

TEST_CASE("can union sets")
{
    struct csp_process_set set1;
    struct csp_process_set set2;
    csp_process_set_init(&set1);
    csp_process_set_add(&set1, &p0);
    csp_process_set_add(&set1, &p1);
    csp_process_set_init(&set2);
    csp_process_set_add(&set2, &p5);
    csp_process_set_union(&set2, &set1);
    check(csp_process_set_eq(&set2, process_set(&p0, &p1, &p5)));
    csp_process_set_done(&set1);
    csp_process_set_done(&set2);
}

TEST_CASE("can compare sets")
{
    struct csp_process_set set1;
    struct csp_process_set set2;
    /* bulk */
    csp_process_set_init(&set1);
    csp_process_set_add(&set1, &p5);
    csp_process_set_add(&set1, &p1);
    /* */
    csp_process_set_init(&set2);
    csp_process_set_add(&set2, &p1);
    csp_process_set_add(&set2, &p5);
    /* compare */
    check(csp_process_set_eq(&set1, &set2));
    /* clean up */
    csp_process_set_done(&set1);
    csp_process_set_done(&set2);
}
