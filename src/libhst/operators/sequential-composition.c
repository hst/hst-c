/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "hst.h"

struct csp_sequential_composition {
    csp_id  p;
    csp_id  q;
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
                                    struct csp_id_set_builder *builder,
                                    void *vseq)
{
    struct csp_sequential_composition  *seq = vseq;
    /* 1) P;Q can perform all of the same events as P, except for ✔.
     * 2) If P can perform ✔, then P;Q can perform τ.
     *
     * initials(P;Q) = initials(P) ∖ {✔}                                [rule 1]
     *               ∪ (✔ ∈ initials(P)? {τ}: {})                       [rule 2]
     */
    csp_process_build_initials(csp, seq->p, builder);
    if (csp_id_set_builder_remove(builder, csp->tick)) {
        csp_id_set_builder_add(builder, csp->tau);
    }
}

static void
csp_sequential_composition_afters(struct csp *csp, csp_id initial,
                                  struct csp_id_set_builder *builder,
                                  void *vseq)
{
    /* afters(P;Q a ≠ ✔) = afters(P, a)                                 [rule 1]
     * afters(P;Q, τ) = Q  if ✔ ∈ initials(P)                           [rule 2]
     *                = {} if ✔ ∉ initials(P)
     * afters(P;Q, ✔) = {}
     *
     * (Note that τ is covered by both rules)
     */
    struct csp_sequential_composition  *seq = vseq;
    size_t  i;
    struct csp_id_set_builder  afters_builder;
    struct csp_id_set  afters;

    /* The composition can never perform a ✔; that's always translated into a τ
     * that activates process Q. */
    if (initial == csp->tick) {
        return;
    }

    /* If P can perform a non-✔ event (including τ) leading to P', then P;Q can
     * also perform that event, leading to P';Q. */
    csp_id_set_builder_init(&afters_builder);
    csp_id_set_init(&afters);
    csp_process_build_afters(csp, seq->p, initial, &afters_builder);
    csp_id_set_build(&afters, &afters_builder);
    for (i = 0; i < afters.count; i++) {
        csp_id  p_prime = afters.ids[i];
        csp_id  seq_prime = csp_sequential_composition(csp, p_prime, seq->q);
        csp_id_set_builder_add(builder, seq_prime);
    }

    /* If P can perform a ✔ leading to P', then P;Q can perform a τ leading to
     * Q.  Note that we don't care what P' is; we just care that it exists. */
    if (initial == csp->tau) {
        csp_process_build_afters(csp, seq->p, csp->tick, &afters_builder);
        csp_id_set_build(&afters, &afters_builder);
        if (afters.count > 0) {
            /* A can perform ✔, and we don't actually care what it leads to,
             * since we're going to lead to Q no matter what. */
            csp_id_set_builder_add(builder, seq->q);
        }
    }

    csp_id_set_builder_done(&afters_builder);
    csp_id_set_done(&afters);
}

static csp_id
csp_sequential_composition_get_id(struct csp *csp, const void *vinput)
{
    const struct csp_sequential_composition  *input = vinput;
    static struct csp_id_scope  sequential_composition;
    csp_id  id = csp_id_start(&sequential_composition);
    id = csp_id_add_id(id, input->p);
    id = csp_id_add_id(id, input->q);
    return id;
}

static size_t
csp_sequential_composition_ud_size(struct csp *csp, const void *vinput)
{
    return sizeof(struct csp_sequential_composition);
}

static void
csp_sequential_composition_init(struct csp *csp, void *vseq, const void *vinput)
{
    struct csp_sequential_composition  *seq = vseq;
    const struct csp_sequential_composition  *input = vinput;
    seq->p = input->p;
    seq->q = input->q;
}

static void
csp_sequential_composition_done(struct csp *csp, void *vseq)
{
    /* nothing to do */
}

static const struct csp_process_iface  csp_sequential_composition_iface = {
    &csp_sequential_composition_initials,
    &csp_sequential_composition_afters,
    &csp_sequential_composition_get_id,
    &csp_sequential_composition_ud_size,
    &csp_sequential_composition_init,
    &csp_sequential_composition_done
};

csp_id
csp_sequential_composition(struct csp *csp, csp_id p, csp_id q)
{
    struct csp_sequential_composition  input = { p, q };
    return csp_process_init(
            csp, &input, NULL, &csp_sequential_composition_iface);
}
