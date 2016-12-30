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

struct csp_external_choice {
    struct csp_id_set ps;
};

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
csp_external_choice_initials(struct csp *csp, struct csp_id_set *set,
                             void *vchoice)
{
    /* 1) If P ∈ Ps can perform τ, then □ Ps can perform τ.
     * 2) If P ∈ Ps can perform a ≠ τ, then □ Ps can perform a ≠ τ.
     *
     * initials(□ Ps) = ⋃ { initials(P) ∩ {τ} | P ∈ Ps }                [rule 1]
     *                ∪ ⋃ { initials(P) ∖ {τ} | P ∈ Ps }                [rule 2]
     *
     *                = ⋃ { initials(P) | P ∈ Ps }
     */
    struct csp_external_choice *choice = vchoice;
    struct csp_id_set_iterator i;
    csp_id_set_foreach (&choice->ps, &i) {
        csp_process_build_initials(csp, csp_id_set_iterator_get(&i), set);
    }
}

static void
csp_external_choice_afters(struct csp *csp, csp_id initial,
                           struct csp_id_set *set, void *vchoice)
{
    /* afters(□ Ps, τ) = ⋃ { □ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
     *                                                                  [rule 1]
     * afters(□ Ps, a ≠ τ) = ⋃ { P' | P ∈ Ps, P' ∈ afters(P, a) }       [rule 2]
     */
    struct csp_external_choice *choice = vchoice;
    if (initial == csp->tau) {
        struct csp_id_set_iterator i;
        /* We'll need to grab afters(P, τ) for each P ∈ Ps. */
        struct csp_id_set p_afters;
        /* We're going to build up a lot of new Ps' sets that all have the same
         * basic structure: Ps' = Ps ∖ {P} ∪ {P'} */
        struct csp_id_set ps_prime;
        csp_id_set_init(&p_afters);
        csp_id_set_init(&ps_prime);
        /* Each Ps' starts with Ps, so go ahead and add that into our Ps' set
         * once. */
        csp_id_set_union(&ps_prime, &choice->ps);
        /* For all P ∈ Ps */
        csp_id_set_foreach (&choice->ps, &i) {
            struct csp_id_set_iterator j;
            csp_id p = csp_id_set_iterator_get(&i);
            /* Set Ps' to Ps ∖ {P} */
            csp_id_set_remove(&ps_prime, p);
            /* Grab afters(P, τ) */
            csp_id_set_clear(&p_afters);
            csp_process_build_afters(csp, p, initial, &p_afters);
            /* For all P' ∈ afters(P, τ) */
            csp_id_set_foreach (&p_afters, &j) {
                csp_id p_prime = csp_id_set_iterator_get(&j);
                /* ps_prime currently contains Ps.  Add P' and remove P to
                 * produce (Ps ∖ {P} ∪ {P'}) */
                csp_id_set_add(&ps_prime, p_prime);
                /* Create □ (Ps ∖ {P} ∪ {P'}) as a result.  Since we'll probably
                 * have to build another similar P' set soon, use the
                 * build_and_keep variant so that we don't throw away all our
                 * work. */
                csp_id_set_add(set,
                               csp_replicated_external_choice(csp, &ps_prime));
                /* Reset Ps' back to Ps ∖ {P}. */
                csp_id_set_remove(&ps_prime, p_prime);
            }
            /* Reset Ps' back to Ps. */
            csp_id_set_add(&ps_prime, p);
        }
        csp_id_set_done(&p_afters);
        csp_id_set_done(&ps_prime);
    } else {
        struct csp_id_set_iterator i;
        csp_id_set_foreach (&choice->ps, &i) {
            csp_id p = csp_id_set_iterator_get(&i);
            csp_process_build_afters(csp, p, initial, set);
        }
    }
}

static csp_id
csp_external_choice_get_id(struct csp *csp, const void *vps)
{
    const struct csp_id_set *ps = vps;
    static struct csp_id_scope external_choice;
    csp_id id = csp_id_start(&external_choice);
    id = csp_id_add_id_set(id, ps);
    return id;
}

static size_t
csp_external_choice_ud_size(struct csp *csp, const void *vps)
{
    return sizeof(struct csp_external_choice);
}

static void
csp_external_choice_init(struct csp *csp, void *vchoice, const void *vps)
{
    struct csp_external_choice *choice = vchoice;
    const struct csp_id_set *ps = vps;
    csp_id_set_init(&choice->ps);
    csp_id_set_union(&choice->ps, ps);
}

static void
csp_external_choice_done(struct csp *csp, void *vchoice)
{
    struct csp_external_choice *choice = vchoice;
    csp_id_set_done(&choice->ps);
}

static const struct csp_process_iface csp_external_choice_iface = {
        &csp_external_choice_initials, &csp_external_choice_afters,
        &csp_external_choice_get_id,   &csp_external_choice_ud_size,
        &csp_external_choice_init,     &csp_external_choice_done};

csp_id
csp_external_choice(struct csp *csp, csp_id a, csp_id b)
{
    csp_id id;
    struct csp_id_set ps;
    csp_id_set_init(&ps);
    csp_id_set_add(&ps, a);
    csp_id_set_add(&ps, b);
    id = csp_process_init(csp, &ps, NULL, &csp_external_choice_iface);
    csp_id_set_done(&ps);
    return id;
}

csp_id
csp_replicated_external_choice(struct csp *csp, const struct csp_id_set *ps)
{
    return csp_process_init(csp, ps, NULL, &csp_external_choice_iface);
}
