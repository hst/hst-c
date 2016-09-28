/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "hst.h"

static struct csp_id_scope  internal_choice;

static csp_id
csp_internal_choice_id(const struct csp_id_set *processes)
{
    csp_id  id = csp_id_start(&internal_choice);
    id = csp_id_add_id_set(id, processes);
    return id;
}

struct csp_internal_choice {
    struct csp_id_set  processes;
};

/* Leaves `processes` unfilled; you have to fill this in yourself. */
static struct csp_internal_choice *
csp_internal_choice_new(void)
{
    struct csp_internal_choice  *choice =
        malloc(sizeof(struct csp_internal_choice));
    assert(choice != NULL);
    csp_id_set_init(&choice->processes);
    return choice;
}

static void
csp_internal_choice_initials(struct csp *csp,
                             struct csp_id_set_builder *builder, void *vchoice)
{
    csp_id_set_builder_add(builder, csp->tau);
}

static void
csp_internal_choice_afters(struct csp *csp, csp_id initial,
                           struct csp_id_set_builder *builder, void *vchoice)
{
    struct csp_internal_choice  *choice = vchoice;
    if (initial == csp->tau) {
        csp_id_set_builder_merge(builder, &choice->processes);
    }
}

static void
csp_internal_choice_free(struct csp *csp, void *vchoice)
{
    struct csp_internal_choice  *choice = vchoice;
    csp_process_deref_set(csp, &choice->processes);
    csp_id_set_done(&choice->processes);
    free(choice);
}

const struct csp_process_iface  csp_internal_choice_iface = {
    &csp_internal_choice_initials,
    &csp_internal_choice_afters,
    &csp_internal_choice_free
};

csp_id
csp_internal_choice(struct csp *csp, csp_id a, csp_id b)
{
    csp_id  id;
    struct csp_internal_choice  *choice;
    choice = csp_internal_choice_new();
    csp_id_set_fill_double(&choice->processes, a, b);
    id = csp_internal_choice_id(&choice->processes);
    csp_process_init(csp, id, choice, &csp_internal_choice_iface);
    return id;
}

csp_id
csp_replicated_internal_choice(struct csp *csp,
                               const struct csp_id_set *processes)
{
    csp_id  id = csp_internal_choice_id(processes);
    struct csp_internal_choice  *choice = csp_internal_choice_new();
    csp_id_set_clone(&choice->processes, processes);
    csp_process_init(csp, id, choice, &csp_internal_choice_iface);
    return id;
}
