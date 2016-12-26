/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"
#include "test-case-harness.h"

#define CSP_ID_SET_FIRST_ALLOCATION_COUNT 32

TEST_CASE_GROUP("identifier sets");

TEST_CASE("can create empty set")
{
    struct csp_id_set set;
    csp_id_set_init(&set);
    check_set_eq(&set, id_set());
    csp_id_set_done(&set);
}

TEST_CASE("can add individual ids")
{
    struct csp_id_set set;
    csp_id_set_init(&set);
    check(csp_id_set_add(&set, 0));
    check(csp_id_set_add(&set, 5));
    check(csp_id_set_add(&set, 1));
    check_set_eq(&set, id_set(0, 1, 5));
    csp_id_set_done(&set);
}

TEST_CASE("can add duplicate individual ids")
{
    struct csp_id_set set;
    csp_id_set_init(&set);
    check(csp_id_set_add(&set, 0));
    check(csp_id_set_add(&set, 5));
    check(csp_id_set_add(&set, 1));
    check(!csp_id_set_add(&set, 5));
    check(!csp_id_set_add(&set, 0));
    check(!csp_id_set_add(&set, 0));
    check_set_eq(&set, id_set(0, 1, 5));
    csp_id_set_done(&set);
}

TEST_CASE("can remove individual ids")
{
    struct csp_id_set set;
    csp_id_set_init(&set);
    csp_id_set_add(&set, 0);
    csp_id_set_add(&set, 5);
    csp_id_set_add(&set, 1);
    check(csp_id_set_remove(&set, 5));
    check(!csp_id_set_remove(&set, 6));
    check_set_eq(&set, id_set(0, 1));
    csp_id_set_done(&set);
}

TEST_CASE("can remove missing individual ids")
{
    struct csp_id_set set;
    csp_id_set_init(&set);
    csp_id_set_add(&set, 0);
    csp_id_set_add(&set, 5);
    csp_id_set_add(&set, 1);
    csp_id_set_remove(&set, 5);
    csp_id_set_remove(&set, 7);
    check_set_eq(&set, id_set(0, 1));
    csp_id_set_done(&set);
}

TEST_CASE("can add bulk ids")
{
    struct csp_id_set set;
    csp_id to_add[] = {0, 5, 1};
    size_t to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    csp_id_set_init(&set);
    csp_id_set_add_many(&set, to_add_count, to_add);
    check_set_eq(&set, id_set(0, 1, 5));
    csp_id_set_done(&set);
}

TEST_CASE("can add duplicate bulk ids")
{
    struct csp_id_set set;
    csp_id to_add[] = {0, 5, 1, 5, 0, 0};
    size_t to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    csp_id_set_init(&set);
    csp_id_set_add_many(&set, to_add_count, to_add);
    check_set_eq(&set, id_set(0, 1, 5));
    csp_id_set_done(&set);
}

TEST_CASE("can remove bulk ids")
{
    struct csp_id_set set;
    csp_id to_add[] = {0, 5, 1, 6};
    size_t to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    csp_id to_remove[] = {1, 5};
    size_t to_remove_count = sizeof(to_remove) / sizeof(to_remove[0]);
    csp_id_set_init(&set);
    csp_id_set_add_many(&set, to_add_count, to_add);
    csp_id_set_remove_many(&set, to_remove_count, to_remove);
    check_set_eq(&set, id_set(0, 6));
    csp_id_set_done(&set);
}

TEST_CASE("can remove missing bulk ids")
{
    struct csp_id_set set;
    csp_id to_add[] = {0, 5, 1, 6};
    size_t to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    csp_id to_remove[] = {1, 7};
    size_t to_remove_count = sizeof(to_remove) / sizeof(to_remove[0]);
    csp_id_set_init(&set);
    csp_id_set_add_many(&set, to_add_count, to_add);
    csp_id_set_remove_many(&set, to_remove_count, to_remove);
    check_set_eq(&set, id_set(0, 5, 6));
    csp_id_set_done(&set);
}

TEST_CASE("can union sets")
{
    struct csp_id_set set1;
    struct csp_id_set set2;
    csp_id_set_init(&set1);
    csp_id_set_add(&set1, 0);
    csp_id_set_add(&set1, 1);
    csp_id_set_init(&set2);
    csp_id_set_add(&set2, 5);
    csp_id_set_union(&set2, &set1);
    check_set_eq(&set2, id_set(0, 1, 5));
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("can clone a set")
{
    struct csp_id_set set1;
    struct csp_id_set set2;
    csp_id to_add[] = {0, 5, 1};
    size_t to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    /* Create a set. */
    csp_id_set_init(&set1);
    csp_id_set_add_many(&set1, to_add_count, to_add);
    /* Then create a copy of it. */
    csp_id_set_init(&set2);
    csp_id_set_union(&set2, &set1);
    /* And verify its contents. */
    check_set_eq(&set2, id_set(0, 1, 5));
    /* Clean up. */
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("can compare sets")
{
    struct csp_id_set set1;
    struct csp_id_set set2;
    csp_id to_add[] = {5, 1};
    size_t to_add_count = sizeof(to_add) / sizeof(to_add[0]);
    /* bulk */
    csp_id_set_init(&set1);
    csp_id_set_add_many(&set1, to_add_count, to_add);
    /* individual */
    csp_id_set_init(&set2);
    csp_id_set_add(&set2, 1);
    csp_id_set_add(&set2, 5);
    /* compare */
    check(csp_id_set_eq(&set1, &set2));
    /* clean up */
    csp_id_set_done(&set1);
    csp_id_set_done(&set2);
}

TEST_CASE("can check subsets")
{
    check(csp_id_set_subseteq(id_set(), id_set()));
    check(csp_id_set_subseteq(id_set(), id_set(1)));
    check(!csp_id_set_subseteq(id_set(5), id_set()));
    check(!csp_id_set_subseteq(id_set(5), id_set(4)));
    check(csp_id_set_subseteq(id_set(5), id_set(4, 5)));
    check(csp_id_set_subseteq(id_set(5), id_set(4, 5, 6)));
    check(csp_id_set_subseteq(id_set(5), id_set(5)));
    check(csp_id_set_subseteq(id_set(5), id_set(5, 6)));
    check(!csp_id_set_subseteq(id_set(5), id_set(6)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set()));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(3)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(3, 4)));
    check(csp_id_set_subseteq(id_set(4, 5), id_set(3, 4, 5)));
    check(csp_id_set_subseteq(id_set(4, 5), id_set(3, 4, 5, 6)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(3, 5)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(4)));
    check(csp_id_set_subseteq(id_set(4, 5), id_set(4, 5)));
    check(csp_id_set_subseteq(id_set(4, 5), id_set(4, 5, 6)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(4, 6)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(5)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(5, 6)));
    check(!csp_id_set_subseteq(id_set(4, 5), id_set(6)));
}
