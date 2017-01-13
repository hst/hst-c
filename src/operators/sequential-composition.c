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

/*------------------------------------------------------------------------------
 * Translate ✔ to τ
 */

struct csp_translate_tick_to_tau {
    struct csp_event_visitor visitor;
    struct csp_event_visitor *wrapped;
};

static void
csp_translate_tick_to_tau_visit(struct csp *csp,
                                struct csp_event_visitor *visitor,
                                const struct csp_event *event)
{
    struct csp_translate_tick_to_tau *self =
            container_of(visitor, struct csp_translate_tick_to_tau, visitor);
    csp_event_visitor_call(csp, self->wrapped,
                           event == csp->tick ? csp->tau : event);
}

struct csp_translate_tick_to_tau
csp_translate_tick_to_tau(struct csp_event_visitor *wrapped)
{
    struct csp_translate_tick_to_tau self = {{csp_translate_tick_to_tau_visit},
                                             wrapped};
    return self;
}

/*------------------------------------------------------------------------------
 * Sequential composition
 */

struct csp_sequential_composition {
    struct csp_process process;
    struct csp_process *p;
    struct csp_process *q;
};

/* Operational semantics for P ; Q
 *
 *        P -a→ P'
 * 1)  ────────────── a ≠ ✔
 *      P;Q -a→ P';Q
 *
 *     ∃ P' • P -✔→ P'
 * 2) ─────────────────
 *       P;Q -τ→ Q
 */

static void
csp_sequential_composition_initials(struct csp *csp,
                                    struct csp_process *process,
                                    struct csp_event_visitor *visitor)
{
    struct csp_sequential_composition *seq =
            container_of(process, struct csp_sequential_composition, process);
    struct csp_translate_tick_to_tau translate;
    /* 1) P;Q can perform all of the same events as P, except for ✔.
     * 2) If P can perform ✔, then P;Q can perform τ.
     *
     * initials(P;Q) = initials(P) ∖ {✔}                                [rule 1]
     *               ∪ (✔ ∈ initials(P)? {τ}: {})                       [rule 2]
     */
    translate = csp_translate_tick_to_tau(visitor);
    csp_process_visit_initials(csp, seq->p, &translate.visitor);
}

static void
csp_sequential_composition_afters(struct csp *csp, struct csp_process *process,
                                  const struct csp_event *initial,
                                  struct csp_edge_visitor *visitor)
{
    /* afters(P;Q a ≠ ✔) = afters(P, a)                                 [rule 1]
     * afters(P;Q, τ) = Q  if ✔ ∈ initials(P)                           [rule 2]
     *                = {} if ✔ ∉ initials(P)
     * afters(P;Q, ✔) = {}
     *
     * (Note that τ is covered by both rules)
     */
    struct csp_sequential_composition *seq =
            container_of(process, struct csp_sequential_composition, process);
    struct csp_process_set_iterator i;
    struct csp_process_set afters;
    struct csp_collect_afters collect = csp_collect_afters(&afters);

    /* The composition can never perform a ✔; that's always translated into a τ
     * that activates process Q. */
    if (initial == csp->tick) {
        return;
    }

    /* If P can perform a non-✔ event (including τ) leading to P', then P;Q can
     * also perform that event, leading to P';Q. */
    csp_process_set_init(&afters);
    csp_process_visit_afters(csp, seq->p, initial, &collect.visitor);
    csp_process_set_foreach (&afters, &i) {
        struct csp_process *p_prime = csp_process_set_iterator_get(&i);
        struct csp_process *seq_prime =
                csp_sequential_composition(csp, p_prime, seq->q);
        csp_edge_visitor_call(csp, visitor, initial, seq_prime);
    }

    /* If P can perform a ✔ leading to P', then P;Q can perform a τ leading to
     * Q.  Note that we don't care what P' is; we just care that it exists. */
    if (initial == csp->tau) {
        csp_process_set_clear(&afters);
        csp_process_visit_afters(csp, seq->p, csp->tick, &collect.visitor);
        if (!csp_process_set_empty(&afters)) {
            /* A can perform ✔, and we don't actually care what it leads to,
             * since we're going to lead to Q no matter what. */
            csp_edge_visitor_call(csp, visitor, initial, seq->q);
        }
    }

    csp_process_set_done(&afters);
}

static void
csp_sequential_composition_free(struct csp *csp, struct csp_process *process)
{
    struct csp_sequential_composition *seq =
            container_of(process, struct csp_sequential_composition, process);
    free(seq);
}

static const struct csp_process_iface csp_sequential_composition_iface = {
        csp_sequential_composition_initials, csp_sequential_composition_afters,
        csp_sequential_composition_free};

static csp_id
csp_sequential_composition_get_id(struct csp_process *p, struct csp_process *q)
{
    static struct csp_id_scope sequential_composition;
    csp_id id = csp_id_start(&sequential_composition);
    id = csp_id_add_process(id, p);
    id = csp_id_add_process(id, q);
    return id;
}

static struct csp_process *
csp_sequential_composition_new(struct csp *csp, struct csp_process *p,
                               struct csp_process *q)
{
    csp_id id = csp_sequential_composition_get_id(p, q);
    struct csp_sequential_composition *seq;
    return_if_nonnull(csp_get_process(csp, id));
    seq = malloc(sizeof(struct csp_sequential_composition));
    assert(seq != NULL);
    seq->process.id = id;
    seq->process.iface = &csp_sequential_composition_iface;
    seq->p = p;
    seq->q = q;
    csp_register_process(csp, &seq->process);
    return &seq->process;
}

struct csp_process *
csp_sequential_composition(struct csp *csp, struct csp_process *p,
                           struct csp_process *q)
{
    return csp_sequential_composition_new(csp, p, q);
}
