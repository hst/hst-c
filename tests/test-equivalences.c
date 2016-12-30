/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "equivalence.h"

#include "test-case-harness.h"
#include "test-cases.h"

TEST_CASE_GROUP("equivalences");

TEST_CASE("can create empty equivalence")
{
    struct csp *csp;
    struct csp_equivalences equiv;
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    check_equivalence_classes(csp, &equiv, ids());
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), id(10));
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), id(20));
    check_equivalence_class_members(csp, &equiv, id(1), ids());
    check_equivalence_class_members(csp, &equiv, id(2), ids());
    csp_equivalences_done(&equiv);
    csp_free(csp);
}

TEST_CASE("can add elements to equivalence")
{
    struct csp *csp;
    struct csp_equivalences equiv;
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    csp_equivalences_add(&equiv, 1, 50);
    csp_equivalences_add(&equiv, 1, 60);
    csp_equivalences_add(&equiv, 1, 70);
    csp_equivalences_add(&equiv, 2, 80);
    check_equivalence_classes(csp, &equiv, ids(1, 2));
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), id(10));
    check_equivalence_class(csp, &equiv, id(1), id(50));
    check_equivalence_class(csp, &equiv, id(1), id(60));
    check_equivalence_class(csp, &equiv, id(1), id(70));
    check_equivalence_class(csp, &equiv, id(2), id(80));
    check_equivalence_class_members(csp, &equiv, id(1), ids(50, 60, 70));
    check_equivalence_class_members(csp, &equiv, id(2), ids(80));
    csp_equivalences_done(&equiv);
    csp_free(csp);
}

TEST_CASE("can updates elements in an equivalence")
{
    struct csp *csp;
    struct csp_equivalences equiv;
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    csp_equivalences_add(&equiv, 1, 50);
    csp_equivalences_add(&equiv, 1, 60);
    csp_equivalences_add(&equiv, 1, 70);
    csp_equivalences_add(&equiv, 2, 50); /* overwrite old class */
    csp_equivalences_add(&equiv, 2, 80);
    check_equivalence_classes(csp, &equiv, ids(1, 2));
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), id(10));
    check_equivalence_class(csp, &equiv, id(1), id(60));
    check_equivalence_class(csp, &equiv, id(1), id(70));
    check_equivalence_class(csp, &equiv, id(2), id(50));
    check_equivalence_class(csp, &equiv, id(2), id(80));
    check_equivalence_class_members(csp, &equiv, id(1), ids(60, 70));
    check_equivalence_class_members(csp, &equiv, id(2), ids(50, 80));
    csp_equivalences_done(&equiv);
    csp_free(csp);
}
