/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "equivalence.h"

#include "test-case-harness.h"
#include "test-cases.h"

static void
check_equivalence_classes(struct csp *csp, struct csp_equivalences *equiv,
                          struct csp_id_set_factory classes)
{
    struct csp_id_set actual;
    const struct csp_id_set *class_set;
    csp_id_set_init(&actual);
    csp_equivalences_build_classes(equiv, &actual);
    class_set = csp_id_set_factory_create(csp, classes);
    check_set_eq(&actual, class_set);
    csp_id_set_done(&actual);
}

static void
check_equivalences_add(struct csp *csp, struct csp_equivalences *equiv,
                       struct csp_id_factory class_id_,
                       struct csp_process_factory process_)
{
    csp_id class_id;
    struct csp_process *process;
    class_id = csp_id_factory_create(csp, class_id_);
    process = csp_process_factory_create(csp, process_);
    csp_equivalences_add(equiv, class_id, process);
}

static void
check_equivalence_class(struct csp *csp, struct csp_equivalences *equiv,
                        struct csp_id_factory class_id_,
                        struct csp_process_factory process_)
{
    csp_id class_id;
    struct csp_process *process;
    csp_id actual;
    class_id = csp_id_factory_create(csp, class_id_);
    process = csp_process_factory_create(csp, process_);
    actual = csp_equivalences_get_class(equiv, process);
    check_id_eq(actual, class_id);
}

static void
check_equivalence_class_members(struct csp *csp, struct csp_equivalences *equiv,
                                struct csp_id_factory class_id_,
                                struct csp_process_set_factory processes_)
{
    csp_id class_id;
    const struct csp_process_set *processes;
    const struct csp_process_set *actual;
    class_id = csp_id_factory_create(csp, class_id_);
    processes = csp_process_set_factory_create(csp, processes_);
    actual = csp_equivalences_get_members(equiv, class_id);
    check_process_set_eq(csp, actual, processes);
}

TEST_CASE_GROUP("equivalences");

TEST_CASE("can create empty equivalence")
{
    struct csp *csp;
    struct csp_equivalences equiv;
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    check_equivalence_classes(csp, &equiv, ids());
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), csp0("a → STOP"));
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), csp0("b → STOP"));
    check_equivalence_class_members(csp, &equiv, id(1), csp0s());
    check_equivalence_class_members(csp, &equiv, id(2), csp0s());
    csp_equivalences_done(&equiv);
    csp_free(csp);
}

TEST_CASE("can add elements to equivalence")
{
    struct csp *csp;
    struct csp_equivalences equiv;
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    check_equivalences_add(csp, &equiv, id(1), csp0("a → STOP"));
    check_equivalences_add(csp, &equiv, id(1), csp0("b → STOP"));
    check_equivalences_add(csp, &equiv, id(1), csp0("c → STOP"));
    check_equivalences_add(csp, &equiv, id(2), csp0("d → STOP"));
    check_equivalence_classes(csp, &equiv, ids(1, 2));
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), csp0("e → STOP"));
    check_equivalence_class(csp, &equiv, id(1), csp0("a → STOP"));
    check_equivalence_class(csp, &equiv, id(1), csp0("b → STOP"));
    check_equivalence_class(csp, &equiv, id(1), csp0("c → STOP"));
    check_equivalence_class(csp, &equiv, id(2), csp0("d → STOP"));
    check_equivalence_class_members(csp, &equiv, id(1),
                                    csp0s("a → STOP", "b → STOP", "c → STOP"));
    check_equivalence_class_members(csp, &equiv, id(2), csp0s("d → STOP"));
    csp_equivalences_done(&equiv);
    csp_free(csp);
}

TEST_CASE("can updates elements in an equivalence")
{
    struct csp *csp;
    struct csp_equivalences equiv;
    check_alloc(csp, csp_new());
    csp_equivalences_init(&equiv);
    check_equivalences_add(csp, &equiv, id(1), csp0("a → STOP"));
    check_equivalences_add(csp, &equiv, id(1), csp0("b → STOP"));
    check_equivalences_add(csp, &equiv, id(1), csp0("c → STOP"));
    /* overwrite old class */
    check_equivalences_add(csp, &equiv, id(2), csp0("a → STOP"));
    check_equivalences_add(csp, &equiv, id(2), csp0("d → STOP"));
    check_equivalence_classes(csp, &equiv, ids(1, 2));
    check_equivalence_class(csp, &equiv, id(CSP_ID_NONE), csp0("e → STOP"));
    check_equivalence_class(csp, &equiv, id(1), csp0("b → STOP"));
    check_equivalence_class(csp, &equiv, id(1), csp0("c → STOP"));
    check_equivalence_class(csp, &equiv, id(2), csp0("a → STOP"));
    check_equivalence_class(csp, &equiv, id(2), csp0("d → STOP"));
    check_equivalence_class_members(csp, &equiv, id(1),
                                    csp0s("b → STOP", "c → STOP"));
    check_equivalence_class_members(csp, &equiv, id(2),
                                    csp0s("a → STOP", "d → STOP"));
    csp_equivalences_done(&equiv);
    csp_free(csp);
}
