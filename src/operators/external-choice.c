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

struct csp_external_choice {
    struct csp_process process;
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
csp_external_choice_initials(struct csp *csp, struct csp_process *process,
                             struct csp_id_set *set)
{
    /* 1) If P ∈ Ps can perform τ, then □ Ps can perform τ.
     * 2) If P ∈ Ps can perform a ≠ τ, then □ Ps can perform a ≠ τ.
     *
     * initials(□ Ps) = ⋃ { initials(P) ∩ {τ} | P ∈ Ps }                [rule 1]
     *                ∪ ⋃ { initials(P) ∖ {τ} | P ∈ Ps }                [rule 2]
     *
     *                = ⋃ { initials(P) | P ∈ Ps }
     */
    struct csp_external_choice *choice =
            container_of(process, struct csp_external_choice, process);
    struct csp_id_set_iterator i;
    csp_id_set_foreach (&choice->ps, &i) {
        csp_build_process_initials(csp, csp_id_set_iterator_get(&i), set);
    }
}

static void
csp_external_choice_afters(struct csp *csp, struct csp_process *process,
                           csp_id initial, struct csp_id_set *set)
{
    /* afters(□ Ps, τ) = ⋃ { □ Ps ∖ {P} ∪ {P'} | P ∈ Ps, P' ∈ afters(P, τ) }
     *                                                                  [rule 1]
     * afters(□ Ps, a ≠ τ) = ⋃ { P' | P ∈ Ps, P' ∈ afters(P, a) }       [rule 2]
     */
    struct csp_external_choice *choice =
            container_of(process, struct csp_external_choice, process);
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
            csp_build_process_afters(csp, p, initial, &p_afters);
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
            csp_build_process_afters(csp, p, initial, set);
        }
    }
}

static void
csp_external_choice_free(struct csp *csp, struct csp_process *process)
{
    struct csp_external_choice *choice =
            container_of(process, struct csp_external_choice, process);
    csp_id_set_done(&choice->ps);
    free(choice);
}

static const struct csp_process_iface csp_external_choice_iface = {
        csp_external_choice_initials, csp_external_choice_afters,
        csp_external_choice_free};

static csp_id
csp_external_choice_get_id(const struct csp_id_set *ps)
{
    static struct csp_id_scope external_choice;
    csp_id id = csp_id_start(&external_choice);
    id = csp_id_add_id_set(id, ps);
    return id;
}

static struct csp_process *
csp_external_choice_new(struct csp *csp, const struct csp_id_set *ps)
{
    csp_id id = csp_external_choice_get_id(ps);
    struct csp_external_choice *choice;
    return_if_nonnull(csp_get_process(csp, id));
    choice = malloc(sizeof(struct csp_external_choice));
    assert(choice != NULL);
    choice->process.id = id;
    choice->process.iface = &csp_external_choice_iface;
    csp_id_set_init(&choice->ps);
    csp_id_set_union(&choice->ps, ps);
    csp_register_process(csp, &choice->process);
    return &choice->process;
}

csp_id
csp_external_choice(struct csp *csp, csp_id a, csp_id b)
{
    struct csp_process *process;
    struct csp_id_set ps;
    csp_id_set_init(&ps);
    csp_id_set_add(&ps, a);
    csp_id_set_add(&ps, b);
    process = csp_external_choice_new(csp, &ps);
    csp_id_set_done(&ps);
    return process->id;
}

csp_id
csp_replicated_external_choice(struct csp *csp, const struct csp_id_set *ps)
{
    return csp_external_choice_new(csp, ps)->id;
}
