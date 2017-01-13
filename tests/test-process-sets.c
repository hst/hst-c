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
