/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "hst.h"

static struct csp_id_scope  external_choice;

static csp_id
csp_external_choice_id(const struct csp_id_set *processes)
{
    csp_id  id = csp_id_start(&external_choice);
    id = csp_id_add_id_set(id, processes);
    return id;
}

struct csp_external_choice {
    struct csp_id_set  processes;
};

/* Leaves `processes` unfilled; you have to fill this in yourself. */
static struct csp_external_choice *
csp_external_choice_new(void)
{
    struct csp_external_choice  *choice =
        malloc(sizeof(struct csp_external_choice));
    assert(choice != NULL);
    csp_id_set_init(&choice->processes);
    return choice;
}

static void
csp_external_choice_initials(struct csp *csp,
                             struct csp_id_set_builder *builder, void *vchoice)
{
    struct csp_external_choice  *choice = vchoice;
    size_t  i;
    for (i = 0; i < choice->processes.count; i++) {
        csp_process_build_initials(csp, choice->processes.ids[i], builder);
    }
}

static void
csp_external_choice_afters(struct csp *csp, csp_id initial,
                           struct csp_id_set_builder *builder, void *vchoice)
{
    struct csp_external_choice  *choice = vchoice;
    /* We want to find which of the `processes` being chosen can perform
     * `initial`, and which process (P') they end up in after doing so.  We have
     * to handle τ events specially, since they do not "resolve" the choice. */
    if (initial == csp->tau) {
        size_t  i;
        struct csp_id_set  afters;
        struct csp_id_set_builder  afters_builder;
        struct csp_id_set  new_processes;
        struct csp_id_set_builder  new_processes_builder;
        csp_id_set_init(&afters);
        csp_id_set_builder_init(&afters_builder);
        csp_id_set_init(&new_processes);
        csp_id_set_builder_init(&new_processes_builder);
        csp_id_set_builder_merge(&new_processes_builder, &choice->processes);
        for (i = 0; i < choice->processes.count; i++) {
            size_t  j;
            csp_id  p = choice->processes.ids[i];
            csp_process_build_afters(csp, p, initial, &afters_builder);
            csp_id_set_build(&afters, &afters_builder);
            for (j = 0; j < afters.count; j++) {
                csp_id  p_prime = afters.ids[j];
                csp_id  choice_prime;
                /* If `initial` is a τ, then it does *not* resolve the choice;
                 * P' is available, but so are all of the other states in Ps.
                 * This means we need to create a transition for
                 *
                 *   □ Ps =τ=> □ (Ps ∖ {P} ∪ {P'})
                 */
                /* new_processes_builder currently contains Ps.  Add P' and
                 * remove P to produce (Ps ∖ {P} ∪ {P'}) */
                csp_id_set_builder_remove(&new_processes_builder, p);
                csp_id_set_builder_add(&new_processes_builder, p_prime);
                /* Create □ (Ps ∖ {P} ∪ {P'}) */
                csp_id_set_build_and_keep(
                        &new_processes, &new_processes_builder);
                csp_process_set_ref(csp, &new_processes);
                choice_prime = csp_replicated_external_choice(
                        csp, &new_processes);
                /* Add the new child process to the overall result. */
                csp_id_set_builder_add(builder, choice_prime);
                /* Reset new_processe_builder so that it contains Ps again. */
                csp_id_set_builder_add(&new_processes_builder, p);
                csp_id_set_builder_remove(&new_processes_builder, p_prime);
            }
            csp_process_set_deref(csp, &afters);
        }
        csp_id_set_done(&afters);
        csp_id_set_builder_done(&afters_builder);
        csp_id_set_done(&new_processes);
        csp_id_set_builder_done(&new_processes_builder);
    } else {
        /* If `initial` is not τ, then it resolves the choice; the other
         * alternatives are no longer available.  We need to create a transition
         * for
         *
         *   □ Ps =E=> P'
         */
        size_t  i;
        for (i = 0; i < choice->processes.count; i++) {
            csp_id  p = choice->processes.ids[i];
            csp_process_build_afters(csp, p, initial, builder);
        }
    }
}

static void
csp_external_choice_free(struct csp *csp, void *vchoice)
{
    struct csp_external_choice  *choice = vchoice;
    csp_process_set_deref(csp, &choice->processes);
    csp_id_set_done(&choice->processes);
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
    csp_id_set_fill_double(&choice->processes, a, b);
    id = csp_external_choice_id(&choice->processes);
    csp_process_init(csp, id, choice, &csp_external_choice_iface);
    return id;
}

csp_id
csp_replicated_external_choice(struct csp *csp,
                               const struct csp_id_set *processes)
{
    csp_id  id = csp_external_choice_id(processes);
    struct csp_external_choice  *choice = csp_external_choice_new();
    csp_id_set_clone(&choice->processes, processes);
    csp_process_init(csp, id, choice, &csp_external_choice_iface);
    return id;
}
