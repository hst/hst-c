/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "environment.h"

#include "event.h"
#include "process.h"
#include "test-case-harness.h"
#include "test-cases.h"

TEST_CASE_GROUP("environments");

TEST_CASE("predefined STOP process exists")
{
    struct csp *csp;
    struct csp_event_set initials;
    struct csp_collect_events collect_initials = csp_collect_events(&initials);
    struct csp_process_set afters;
    struct csp_collect_afters collect_afters = csp_collect_afters(&afters);
    /* Create the CSP environment. */
    csp_event_set_init(&initials);
    csp_process_set_init(&afters);
    check_alloc(csp, csp_new());
    /* Verify the initials set of the STOP process. */
    csp_event_set_clear(&initials);
    csp_process_visit_initials(csp, csp->stop, &collect_initials.visitor);
    check(csp_event_set_eq(&initials, event_set()));
    /* Verify the afters of τ. */
    csp_process_set_clear(&afters);
    csp_process_visit_afters(csp, csp->stop, csp->tau, &collect_afters.visitor);
    check(csp_process_set_eq(&afters, process_set()));
    /* Verify the afters of ✔. */
    csp_process_set_clear(&afters);
    csp_process_visit_afters(csp, csp->stop, csp->tick,
                             &collect_afters.visitor);
    check(csp_process_set_eq(&afters, process_set()));
    /* Clean up. */
    csp_event_set_done(&initials);
    csp_process_set_done(&afters);
    csp_free(csp);
}

TEST_CASE("predefined SKIP process exists")
{
    struct csp *csp;
    struct csp_event_set initials;
    struct csp_collect_events collect = csp_collect_events(&initials);
    struct csp_process_set afters;
    struct csp_collect_afters collect_afters = csp_collect_afters(&afters);
    /* Create the CSP environment. */
    csp_event_set_init(&initials);
    csp_process_set_init(&afters);
    check_alloc(csp, csp_new());
    /* Verify the initials set of the SKIP process. */
    csp_event_set_clear(&initials);
    csp_process_visit_initials(csp, csp->skip, &collect.visitor);
    check(csp_event_set_eq(&initials, event_set("✔")));
    /* Verify the afters of τ. */
    csp_process_set_clear(&afters);
    csp_process_visit_afters(csp, csp->skip, csp->tau, &collect_afters.visitor);
    check(csp_process_set_eq(&afters, process_set()));
    /* Verify the afters of ✔. */
    csp_process_set_clear(&afters);
    csp_process_visit_afters(csp, csp->skip, csp->tick,
                             &collect_afters.visitor);
    check(csp_process_set_eq(&afters, process_set(csp->stop)));
    /* Clean up. */
    csp_event_set_done(&initials);
    csp_process_set_done(&afters);
    csp_free(csp);
}

TEST_CASE("base process IDs should be reproducible")
{
    static struct csp_id_scope scope;
    check_id_eq(csp_id_start(&scope), csp_id_start(&scope));
}

TEST_CASE("distinct scopes should produce different IDs")
{
    static struct csp_id_scope scope1;
    static struct csp_id_scope scope2;
    check_id_ne(csp_id_start(&scope1), csp_id_start(&scope2));
}

TEST_CASE("ID-derived process IDs should be reproducible")
{
    static struct csp_id_scope scope;
    csp_id base = csp_id_start(&scope);
    check_id_eq(csp_id_add_id(base, 0), csp_id_add_id(base, 0));
    check_id_eq(csp_id_add_id(base, 10), csp_id_add_id(base, 10));
    check_id_eq(csp_id_add_id(base, 100), csp_id_add_id(base, 100));
}

TEST_CASE("distinct IDs should produce different IDs")
{
    static struct csp_id_scope scope;
    csp_id base = csp_id_start(&scope);
    check_id_ne(csp_id_add_id(base, 0), csp_id_add_id(base, 10));
    check_id_ne(csp_id_add_id(base, 0), csp_id_add_id(base, 100));
    check_id_ne(csp_id_add_id(base, 10), csp_id_add_id(base, 100));
}

TEST_CASE("set-derived process IDs should be reproducible")
{
    static struct csp_id_scope scope;
    struct csp_id_set *set1;
    struct csp_id_set *set2;
    csp_id base = csp_id_start(&scope);
    set1 = id_set(1, 2, 3, 4);
    set2 = id_set(1, 2, 3, 4);
    check_id_eq(csp_id_add_id_set(base, set1), csp_id_add_id_set(base, set1));
    check_id_eq(csp_id_add_id_set(base, set1), csp_id_add_id_set(base, set2));
}

TEST_CASE("distinct sets should produce different IDs")
{
    static struct csp_id_scope scope;
    struct csp_id_set *set1;
    struct csp_id_set *set2;
    csp_id base = csp_id_start(&scope);
    set1 = id_set(1, 2, 3, 4);
    set2 = id_set(5, 6, 7, 8);
    check_id_ne(csp_id_add_id_set(base, set1), csp_id_add_id_set(base, set2));
}

TEST_CASE("name-derived process IDs should be reproducible")
{
    static struct csp_id_scope scope;
    csp_id base = csp_id_start(&scope);
    check_id_eq(csp_id_add_name(base, "a"), csp_id_add_name(base, "a"));
    check_id_eq(csp_id_add_name(base, "b"), csp_id_add_name(base, "b"));
    check_id_eq(csp_id_add_name(base, "c"), csp_id_add_name(base, "c"));
}

TEST_CASE("distinct names should produce different IDs")
{
    static struct csp_id_scope scope;
    csp_id base = csp_id_start(&scope);
    check_id_ne(csp_id_add_name(base, "a"), csp_id_add_name(base, "b"));
    check_id_ne(csp_id_add_name(base, "a"), csp_id_add_name(base, "c"));
    check_id_ne(csp_id_add_name(base, "b"), csp_id_add_name(base, "c"));
}
