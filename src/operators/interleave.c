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

struct csp_interleave {
    struct csp_process process;
    struct csp_process_bag ps;
};

/* Operational semantics for ⊓ Ps
 *
 *                  P -τ→ P'
 *  1)  ────────────────────────────── P ∈ Ps
 *       ⫴ Ps -τ→ ⫴ (Ps ∖ {P} ∪ {P'})
 *
 *                  P -a→ P'
 *  2)  ────────────────────────────── P ∈ Ps, a ∉ {τ,✔}
 *       ⫴ Ps -a→ ⫴ (Ps ∖ {P} ∪ {P'})
 *
 *                  P -✔→ P'
 *  3)  ──────────────────────────────── P ∈ Ps
 *       ⫴ Ps -τ→ ⫴ (Ps ∖ {P} ∪ {STOP})
 *
 *  4)  ───────────────────
 *       ⫴ {STOP} -✔→ STOP
 */

static void
csp_interleave_name(struct csp *csp, struct csp_process *process,
                    struct csp_name_visitor *visitor)
{
    struct csp_interleave *interleave =
            container_of(process, struct csp_interleave, process);
    csp_process_bag_nested_name(csp, process, &interleave->ps, "⫴", visitor);
}

static void
csp_interleave_initials(struct csp *csp, struct csp_process *process,
                        struct csp_event_visitor *visitor)
{
    /* initials(⫴ Ps) = ⋃ { initials(P) ∩ {τ} | P ∈ Ps }                [rule 1]
     *                ∪ ⋃ { initials(P) ∖ {τ,✔} | P ∈ Ps }              [rule 2]
     *                ∪ ⋃ { (✔ ∈ initials(P)? {τ}: {}) | P ∈ Ps }       [rule 3]
     *                ∪ (Ps = {STOP}? {✔}: {})                          [rule 4]
     */
    struct csp_interleave *interleave =
            container_of(process, struct csp_interleave, process);
    struct csp_event_set initials;
    struct csp_collect_events collect;
    struct csp_process_bag_iterator i;
    csp_event_set_init(&initials);
    collect = csp_collect_events(&initials);

    /* Rules 1 and 2 */
    csp_process_bag_foreach (&interleave->ps, &i) {
        struct csp_process *p = csp_process_bag_iterator_get(&i);
        csp_process_visit_initials(csp, p, &collect.visitor);
    }
    /* Rule 3 */
    if (csp_event_set_remove(&initials, csp->tick)) {
        csp_event_set_add(&initials, csp->tau);
    }
    /* Rule 4 */
    if (csp_event_set_empty(&initials)) {
        csp_event_set_add(&initials, csp->tick);
    }

    csp_event_set_visit(csp, &initials, visitor);
    csp_event_set_done(&initials);
}

static void
csp_interleave_normal_afters(struct csp *csp, struct csp_process *process,
                             const struct csp_event *initial,
                             struct csp_edge_visitor *visitor)
{
    struct csp_interleave *interleave =
            container_of(process, struct csp_interleave, process);
    /* afters(⫴ Ps, a ∉ {τ,✔}) = ⋃ { ⫴ Ps ∖ {P} ∪ {P'} |
     *                                  P ∈ Ps, P' ∈ afters(P, a) }     [rule 2]
     */
    struct csp_process_bag_iterator i;
    /* We'll need to grab afters(P, a) for each P ∈ Ps. */
    struct csp_process_set p_afters;
    /* We're going to build up a lot of new Ps' sets that all have the same
     * basic structure: Ps' = Ps ∖ {P} ∪ {P'} */
    struct csp_process_bag ps_prime;
    csp_process_set_init(&p_afters);
    csp_process_bag_init(&ps_prime);
    /* Each Ps' starts with Ps, so go ahead and add that to our Ps' set once. */
    csp_process_bag_union(&ps_prime, &interleave->ps);
    /* For all P ∈ Ps */
    csp_process_bag_foreach (&interleave->ps, &i) {
        struct csp_process_set_iterator j;
        struct csp_process *p = csp_process_bag_iterator_get(&i);
        struct csp_collect_afters collect = csp_collect_afters(&p_afters);
        /* Set Ps' to Ps ∖ {P} */
        csp_process_bag_remove(&ps_prime, p);
        /* Grab afters(P, a) */
        csp_process_set_clear(&p_afters);
        csp_process_visit_afters(csp, p, initial, &collect.visitor);
        /* For all P' ∈ afters(P, a) */
        csp_process_set_foreach (&p_afters, &j) {
            struct csp_process *p_prime = csp_process_set_iterator_get(&j);
            /* ps_prime currently contains Ps.  Add P' and remove P to produce
             * (Ps ∖ {P} ∪ {P'}) */
            csp_process_bag_add(&ps_prime, p_prime);
            /* Create ⫴ (Ps ∖ {P} ∪ {P'}) as a result. */
            csp_edge_visitor_call(csp, visitor, initial,
                                  csp_interleave(csp, &ps_prime));
            /* Reset Ps' back to Ps ∖ {P}. */
            csp_process_bag_remove(&ps_prime, p_prime);
        }
        /* Reset Ps' back to Ps. */
        csp_process_bag_add(&ps_prime, p);
    }
    csp_process_set_done(&p_afters);
    csp_process_bag_done(&ps_prime);
}

static void
csp_interleave_tau_for_tick_afters(struct csp *csp, struct csp_process *process,
                                   const struct csp_event *initial,
                                   struct csp_edge_visitor *visitor)
{
    struct csp_interleave *interleave =
            container_of(process, struct csp_interleave, process);
    struct csp_process_bag_iterator i;
    /* We're going to build up a lot of new Ps' sets that all have the same
     * basic structure: Ps' = Ps ∖ {P} ∪ {STOP} */
    struct csp_process_bag ps_prime;
    csp_process_bag_init(&ps_prime);
    /* Each Ps' starts with Ps, so go ahead and add that to our Ps' set once. */
    csp_process_bag_union(&ps_prime, &interleave->ps);
    /* Find each P ∈ Ps where ✔ ∈ initials(P). */
    csp_process_bag_foreach (&interleave->ps, &i) {
        struct csp_process *p = csp_process_bag_iterator_get(&i);
        struct csp_contains_event contains = csp_contains_event(csp->tick);
        csp_process_visit_initials(csp, p, &contains.visitor);
        if (contains.is_present) {
            /* Create Ps ∖ {P} ∪ {STOP}) as a result. */
            csp_process_bag_remove(&ps_prime, p);
            csp_process_bag_add(&ps_prime, csp->stop);
            csp_edge_visitor_call(csp, visitor, initial,
                                  csp_interleave(csp, &ps_prime));
            /* Reset Ps' back to Ps. */
            csp_process_bag_remove(&ps_prime, csp->stop);
            csp_process_bag_add(&ps_prime, p);
        }
    }
    csp_process_bag_done(&ps_prime);
}

static void
csp_interleave_tau_afters(struct csp *csp, struct csp_process *process,
                          const struct csp_event *initial,
                          struct csp_edge_visitor *visitor)
{
    /* afters(⫴ Ps, τ) = ⋃ { ⫴ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
     *                                                                  [rule 1]
     *                 ∪ ⋃ { ⫴ Ps ∖ {P} ∪ {STOP} | P ∈ Ps, P' ∈ afters(P, ✔) }
     *                                                                  [rule 3]
     */
    /* Rule 1 has the same form as rule 2, which we've implemented above.*/
    csp_interleave_normal_afters(csp, process, initial, visitor);
    /* Rule 3...does not. */
    csp_interleave_tau_for_tick_afters(csp, process, initial, visitor);
}

static void
csp_interleave_tick_afters(struct csp *csp, struct csp_process *process,
                           const struct csp_event *initial,
                           struct csp_edge_visitor *visitor)
{
    struct csp_interleave *interleave =
            container_of(process, struct csp_interleave, process);
    /* afters(⫴ {STOP}, ✔) = {STOP}                                  [rule 4] */
    struct csp_any_events any = csp_any_events();
    struct csp_process_bag_iterator i;
    csp_process_bag_foreach (&interleave->ps, &i) {
        struct csp_process *p = csp_process_bag_iterator_get(&i);
        csp_process_visit_initials(csp, p, &any.visitor);
        if (any.has_events) {
            /* One of the subprocess has at least one initial, so this cannot
             * possibly be ⫴ {STOP}. */
            return;
        }
    }
    csp_edge_visitor_call(csp, visitor, initial, csp->stop);
}

static void
csp_interleave_afters(struct csp *csp, struct csp_process *process,
                      const struct csp_event *initial,
                      struct csp_edge_visitor *visitor)
{
    if (initial == csp->tau) {
        csp_interleave_tau_afters(csp, process, initial, visitor);
    } else if (initial == csp->tick) {
        csp_interleave_tick_afters(csp, process, initial, visitor);
    } else {
        csp_interleave_normal_afters(csp, process, initial, visitor);
    }
}

static void
csp_interleave_free(struct csp *csp, struct csp_process *process)
{
    struct csp_interleave *interleave =
            container_of(process, struct csp_interleave, process);
    csp_process_bag_done(&interleave->ps);
    free(interleave);
}

static const struct csp_process_iface csp_interleave_iface = {
        9, csp_interleave_name, csp_interleave_initials, csp_interleave_afters,
        csp_interleave_free};

static csp_id
csp_interleave_get_id(const struct csp_process_bag *ps)
{
    static struct csp_id_scope interleave;
    csp_id id = csp_id_start(&interleave);
    id = csp_id_add_process_bag(id, ps);
    return id;
}

static struct csp_process *
csp_interleave_new(struct csp *csp, const struct csp_process_bag *ps)
{
    csp_id id = csp_interleave_get_id(ps);
    struct csp_interleave *interleave;
    return_if_nonnull(csp_get_process(csp, id));
    interleave = malloc(sizeof(struct csp_interleave));
    assert(interleave != NULL);
    interleave->process.id = id;
    interleave->process.iface = &csp_interleave_iface;
    csp_process_bag_init(&interleave->ps);
    csp_process_bag_union(&interleave->ps, ps);
    csp_register_process(csp, &interleave->process);
    return &interleave->process;
}

struct csp_process *
csp_interleave(struct csp *csp, const struct csp_process_bag *ps)
{
    return csp_interleave_new(csp, ps);
}
