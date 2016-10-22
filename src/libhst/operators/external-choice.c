/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "ccan/likely/likely.h"
#include "hst.h"

static struct csp_id_scope  external_choice;

static csp_id
csp_external_choice_id(const struct csp_id_set *ps)
{
    csp_id  id = csp_id_start(&external_choice);
    id = csp_id_add_id_set(id, ps);
    return id;
}

struct csp_external_choice {
    struct csp_id_set  ps;
};

/* Leaves `ps` unfilled; you have to fill this in yourself. */
static struct csp_external_choice *
csp_external_choice_new(void)
{
    struct csp_external_choice  *choice =
        malloc(sizeof(struct csp_external_choice));
    assert(choice != NULL);
    csp_id_set_init(&choice->ps);
    return choice;
}

/* Operational semantics for □ Ps
 *
 *                  P -τ→ P'
 *  1)  ────────────────────────────── P ∈ Ps
 *       □ Ps -τ→ □ (Ps ∖ {P} ∪ {P'})
 *
 *         P -a→ P'
 *  2)  ───────────── P ∈ Ps, a ≠ τ
 *       □ Ps -a→ P'
 */

static void
csp_external_choice_initials(struct csp *csp,
                             struct csp_id_set_builder *builder, void *vchoice)
{
    /* 1) If P ∈ Ps can perform τ, then □ Ps can perform τ.
     * 2) If P ∈ Ps can perform a ≠ τ, then □ Ps can perform a ≠ τ.
     *
     * initials(□ Ps) = ⋃ { initials(P) ∩ {τ} | P ∈ Ps }                [rule 1]
     *                ∪ ⋃ { initials(P) ∖ {τ} | P ∈ Ps }                [rule 2]
     *
     *                = ⋃ { initials(P) | P ∈ Ps }
     */
    struct csp_external_choice  *choice = vchoice;
    size_t  i;
    for (i = 0; i < choice->ps.count; i++) {
        csp_process_build_initials(csp, choice->ps.ids[i], builder);
    }
}

static void
csp_external_choice_afters(struct csp *csp, csp_id initial,
                           struct csp_id_set_builder *builder, void *vchoice)
{
    /* afters(□ Ps, τ) = ⋃ { □ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
     *                                                                  [rule 1]
     * afters(□ Ps, a ≠ τ) = ⋃ { P' | P ∈ Ps, P' ∈ afters(P, a) }       [rule 2]
     */
    struct csp_external_choice  *choice = vchoice;
    if (initial == csp->tau) {
        size_t  i;
        /* We'll need to grab afters(P, τ) for each P ∈ Ps. */
        struct csp_id_set  p_afters;
        struct csp_id_set_builder  p_afters_builder;
        /* We're going to build up a lot of new Ps' sets that all have the same
         * basic structure: Ps' = Ps ∖ {P} ∪ {P'} */
        struct csp_id_set  ps_prime;
        struct csp_id_set_builder  ps_prime_builder;
        csp_id_set_init(&p_afters);
        csp_id_set_builder_init(&p_afters_builder);
        csp_id_set_init(&ps_prime);
        csp_id_set_builder_init(&ps_prime_builder);
        /* Each Ps' starts with Ps, so go ahead and add that into our Ps'
         * builder once. */
        csp_id_set_builder_merge(&ps_prime_builder, &choice->ps);
        /* For all P ∈ Ps */
        for (i = 0; i < choice->ps.count; i++) {
            size_t  j;
            csp_id  p = choice->ps.ids[i];
            /* Set Ps' to Ps ∖ {P} */
            csp_id_set_builder_remove(&ps_prime_builder, p);
            /* Grab afters(P, τ) */
            csp_process_build_afters(csp, p, initial, &p_afters_builder);
            csp_id_set_build(&p_afters, &p_afters_builder);
            /* For all P' ∈ afters(P, τ) */
            for (j = 0; j < p_afters.count; j++) {
                csp_id  p_prime = p_afters.ids[j];
                /* ps_prime_builder currently contains Ps.  Add P' and
                 * remove P to produce (Ps ∖ {P} ∪ {P'}) */
                csp_id_set_builder_add(&ps_prime_builder, p_prime);
                /* Create □ (Ps ∖ {P} ∪ {P'}) as a result.  Since we'll probably
                 * have to build another similar P' set soon, use the
                 * build_and_keep variant so that we don't throw away all our
                 * work. */
                csp_id_set_build_and_keep(&ps_prime, &ps_prime_builder);
                csp_id_set_builder_add(builder,
                        csp_replicated_external_choice(csp, &ps_prime));
                /* Reset Ps' back to Ps ∖ {P}. */
                csp_id_set_builder_remove(&ps_prime_builder, p_prime);
            }
            /* Reset Ps' back to Ps. */
            csp_id_set_builder_add(&ps_prime_builder, p);
        }
        csp_id_set_done(&p_afters);
        csp_id_set_builder_done(&p_afters_builder);
        csp_id_set_done(&ps_prime);
        csp_id_set_builder_done(&ps_prime_builder);
    } else {
        size_t  i;
        for (i = 0; i < choice->ps.count; i++) {
            csp_id  p = choice->ps.ids[i];
            csp_process_build_afters(csp, p, initial, builder);
        }
    }
}

static void
csp_external_choice_free(struct csp *csp, void *vchoice)
{
    struct csp_external_choice  *choice = vchoice;
    csp_id_set_done(&choice->ps);
    free(choice);
}

const struct csp_process_iface  csp_external_choice_iface = {
    &csp_external_choice_initials,
    &csp_external_choice_afters,
    &csp_external_choice_free
};

csp_id
csp_external_choice(struct csp *csp, csp_id a, csp_id b)
{
    csp_id  id;
    struct csp_external_choice  *choice;
    choice = csp_external_choice_new();
    csp_id_set_fill_double(&choice->ps, a, b);
    id = csp_external_choice_id(&choice->ps);
    csp_process_init(csp, id, choice, &csp_external_choice_iface);
    return id;
}

csp_id
csp_replicated_external_choice(struct csp *csp, const struct csp_id_set *ps)
{
    csp_id  id = csp_external_choice_id(ps);
    struct csp_external_choice  *choice = csp_external_choice_new();
    csp_id_set_clone(&choice->ps, ps);
    csp_process_init(csp, id, choice, &csp_external_choice_iface);
    return id;
}
