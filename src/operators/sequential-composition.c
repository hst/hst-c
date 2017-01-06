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
#include "macros.h"
#include "process.h"

struct csp_sequential_composition {
    struct csp_process process;
    csp_id p;
    csp_id q;
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
                                    struct csp_id_set *set)
{
    struct csp_sequential_composition *seq =
            container_of(process, struct csp_sequential_composition, process);
    /* 1) P;Q can perform all of the same events as P, except for ✔.
     * 2) If P can perform ✔, then P;Q can perform τ.
     *
     * initials(P;Q) = initials(P) ∖ {✔}                                [rule 1]
     *               ∪ (✔ ∈ initials(P)? {τ}: {})                       [rule 2]
     */
    csp_build_process_initials(csp, seq->p, set);
    if (csp_id_set_remove(set, csp->tick)) {
        csp_id_set_add(set, csp->tau);
    }
}

static void
csp_sequential_composition_afters(struct csp *csp, struct csp_process *process,
                                  csp_id initial, struct csp_id_set *set)
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
    struct csp_id_set_iterator i;
    struct csp_id_set afters;

    /* The composition can never perform a ✔; that's always translated into a τ
     * that activates process Q. */
    if (initial == csp->tick) {
        return;
    }

    /* If P can perform a non-✔ event (including τ) leading to P', then P;Q can
     * also perform that event, leading to P';Q. */
    csp_id_set_init(&afters);
    csp_build_process_afters(csp, seq->p, initial, &afters);
    csp_id_set_foreach (&afters, &i) {
        csp_id p_prime = csp_id_set_iterator_get(&i);
        csp_id seq_prime = csp_sequential_composition(csp, p_prime, seq->q);
        csp_id_set_add(set, seq_prime);
    }

    /* If P can perform a ✔ leading to P', then P;Q can perform a τ leading to
     * Q.  Note that we don't care what P' is; we just care that it exists. */
    if (initial == csp->tau) {
        csp_id_set_clear(&afters);
        csp_build_process_afters(csp, seq->p, csp->tick, &afters);
        if (!csp_id_set_empty(&afters)) {
            /* A can perform ✔, and we don't actually care what it leads to,
             * since we're going to lead to Q no matter what. */
            csp_id_set_add(set, seq->q);
        }
    }

    csp_id_set_done(&afters);
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
csp_sequential_composition_get_id(csp_id p, csp_id q)
{
    static struct csp_id_scope sequential_composition;
    csp_id id = csp_id_start(&sequential_composition);
    id = csp_id_add_id(id, p);
    id = csp_id_add_id(id, q);
    return id;
}

static struct csp_process *
csp_sequential_composition_new(struct csp *csp, csp_id p, csp_id q)
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

csp_id
csp_sequential_composition(struct csp *csp, csp_id p, csp_id q)
{
    return csp_sequential_composition_new(csp, p, q)->id;
}
