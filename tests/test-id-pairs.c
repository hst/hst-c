/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "id-pair.h"

#include "test-case-harness.h"

TEST_CASE_GROUP("ID pair sets");

#define make_pair_set(...) \
    csp_id_pair_set_factory_create(NULL, pair_set(__VA_ARGS__))

TEST_CASE("can create empty sets via factory")
{
    struct csp_id_pair_set *set = make_pair_set();
    check(csp_id_pair_set_size(set) == 0);
}

TEST_CASE("can create 1-element set via factory")
{
    struct csp_id_pair_set *set = make_pair_set(id(10), id(20));
    check(csp_id_pair_set_size(set) == 1);
    check(csp_id_pair_set_contains(set, pair(10, 20)));
}

TEST_CASE("can create 5-element set via factory")
{
    struct csp_id_pair_set *set =
            make_pair_set(id(10), id(15), id(10), id(25), id(30), id(35),
                          id(40), id(45), id(50), id(55));
    check(csp_id_pair_set_size(set) == 5);
    check(csp_id_pair_set_contains(set, pair(10, 15)));
    check(csp_id_pair_set_contains(set, pair(10, 25)));
    check(csp_id_pair_set_contains(set, pair(30, 35)));
    check(csp_id_pair_set_contains(set, pair(40, 45)));
    check(csp_id_pair_set_contains(set, pair(50, 55)));
}

TEST_CASE("duplicates are ignored when creating set via factory")
{
    struct csp_id_pair_set *set =
            make_pair_set(id(10), id(15), id(10), id(15), id(30), id(35),
                          id(40), id(45), id(50), id(55));
    check(csp_id_pair_set_size(set) == 4);
    check(csp_id_pair_set_contains(set, pair(10, 15)));
    check(csp_id_pair_set_contains(set, pair(30, 35)));
    check(csp_id_pair_set_contains(set, pair(40, 45)));
    check(csp_id_pair_set_contains(set, pair(50, 55)));
}

static void
check_union(struct csp_id_pair_set_factory lhs,
            struct csp_id_pair_set_factory rhs,
            struct csp_id_pair_set_factory expected)
{
    struct csp_id_pair_set *lhs_set = csp_id_pair_set_factory_create(NULL, lhs);
    struct csp_id_pair_set *rhs_set = csp_id_pair_set_factory_create(NULL, rhs);
    struct csp_id_pair_set *expected_set =
            csp_id_pair_set_factory_create(NULL, expected);
    struct csp_id_pair_set_iterator iter;
    csp_id_pair_set_union(lhs_set, rhs_set);
    check(csp_id_pair_set_size(lhs_set) == csp_id_pair_set_size(expected_set));
    csp_id_pair_set_foreach (expected_set, &iter) {
        const struct csp_id_pair *pair = csp_id_pair_set_iterator_get(&iter);
        check(csp_id_pair_set_contains(lhs_set, *pair));
    }
}

TEST_CASE("can union two sets")
{
    check_union(pair_set(id(10), id(15)), pair_set(id(20), id(25)),
                pair_set(id(10), id(15), id(20), id(25)));
}

TEST_CASE("duplicates are ignored when unioning sets")
{
    check_union(pair_set(id(10), id(15), id(20), id(25)),
                pair_set(id(20), id(25), id(30), id(35)),
                pair_set(id(10), id(15), id(20), id(25), id(30), id(35)));
}

TEST_CASE("can union longer left set")
{
    check_union(pair_set(id(10), id(15), id(20), id(25)),
                pair_set(id(30), id(35)),
                pair_set(id(10), id(15), id(20), id(25), id(30), id(35)));
}

TEST_CASE("can union longer right set")
{
    check_union(pair_set(id(10), id(15)),
                pair_set(id(20), id(25), id(30), id(35)),
                pair_set(id(10), id(15), id(20), id(25), id(30), id(35)));
}
