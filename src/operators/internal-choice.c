/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "operators.h"

#include <assert.h>
#include <stdlib.h>

#include "ccan/container_of/container_of.h"
#include "basics.h"
#include "environment.h"
#include "event.h"
#include "macros.h"
#include "process.h"

struct csp_internal_choice {
    struct csp_process process;
    struct csp_process_set ps;
};

/* Operational semantics for ⊓ Ps
 *
 * 1) ──────────── P ∈ Ps
 *     ⊓ Ps -τ→ P
 */

static void
csp_internal_choice_name(struct csp *csp, struct csp_process *process,
                         struct csp_name_visitor *visitor)
{
    struct csp_internal_choice *choice =
            container_of(process, struct csp_internal_choice, process);
    csp_process_set_nested_name(csp, process, &choice->ps, "⊓", visitor);
}

static void
csp_internal_choice_initials(struct csp *csp, struct csp_process *process,
                             struct csp_event_visitor *visitor)
{
    /* initials(⊓ Ps) = {τ} */
    csp_event_visitor_call(csp, visitor, csp->tau);
}

static void
csp_internal_choice_afters(struct csp *csp, struct csp_process *process,
                           const struct csp_event *initial,
                           struct csp_edge_visitor *visitor)
{
    /* afters(⊓ Ps, τ) = Ps */
    struct csp_internal_choice *choice =
            container_of(process, struct csp_internal_choice, process);
    if (initial == csp->tau) {
        struct csp_process_set_iterator iter;
        csp_process_set_foreach (&choice->ps, &iter) {
            struct csp_process *p = csp_process_set_iterator_get(&iter);
            csp_edge_visitor_call(csp, visitor, initial, p);
        }
    }
}

static void
csp_internal_choice_free(struct csp *csp, struct csp_process *process)
{
    struct csp_internal_choice *choice =
            container_of(process, struct csp_internal_choice, process);
    csp_process_set_done(&choice->ps);
    free(choice);
}

static const struct csp_process_iface csp_internal_choice_iface = {
        7, csp_internal_choice_name, csp_internal_choice_initials,
        csp_internal_choice_afters, csp_internal_choice_free};

static csp_id
csp_internal_choice_get_id(const struct csp_process_set *ps)
{
    static struct csp_id_scope internal_choice;
    csp_id id = csp_id_start(&internal_choice);
    id = csp_id_add_process_set(id, ps);
    return id;
}

static struct csp_process *
csp_internal_choice_new(struct csp *csp, const struct csp_process_set *ps)
{
    csp_id id = csp_internal_choice_get_id(ps);
    struct csp_internal_choice *choice;
    return_if_nonnull(csp_get_process(csp, id));
    choice = malloc(sizeof(struct csp_internal_choice));
    assert(choice != NULL);
    choice->process.id = id;
    choice->process.iface = &csp_internal_choice_iface;
    csp_process_set_init(&choice->ps);
    csp_process_set_union(&choice->ps, ps);
    csp_register_process(csp, &choice->process);
    return &choice->process;
}

struct csp_process *
csp_internal_choice(struct csp *csp, struct csp_process *p,
                    struct csp_process *q)
{
    struct csp_process *process;
    struct csp_process_set ps;
    csp_process_set_init(&ps);
    csp_process_set_add(&ps, p);
    csp_process_set_add(&ps, q);
    process = csp_internal_choice_new(csp, &ps);
    csp_process_set_done(&ps);
    return process;
}

struct csp_process *
csp_replicated_internal_choice(struct csp *csp,
                               const struct csp_process_set *ps)
{
    return csp_internal_choice_new(csp, ps);
}
