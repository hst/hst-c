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

static struct csp_id_scope  internal_choice;

static csp_id
csp_internal_choice_id(const struct csp_id_set *ps)
{
    csp_id  id = csp_id_start(&internal_choice);
    id = csp_id_add_id_set(id, ps);
    return id;
}

struct csp_internal_choice {
    struct csp_id_set  ps;
};

/* Leaves `ps` unfilled; you have to fill this in yourself. */
static struct csp_internal_choice *
csp_internal_choice_new(void)
{
    struct csp_internal_choice  *choice =
        malloc(sizeof(struct csp_internal_choice));
    assert(choice != NULL);
    csp_id_set_init(&choice->ps);
    return choice;
}

/* Operational semantics for ⊓ Ps
 *
 * 1) ──────────── P ∈ Ps
 *     ⊓ Ps -τ→ P
 */

static void
csp_internal_choice_initials(struct csp *csp,
                             struct csp_id_set_builder *builder, void *vchoice)
{
    /* initials(⊓ Ps) = {τ} */
    csp_id_set_builder_add(builder, csp->tau);
}

static void
csp_internal_choice_afters(struct csp *csp, csp_id initial,
                           struct csp_id_set_builder *builder, void *vchoice)
{
    /* afters(⊓ Ps, τ) = Ps */
    struct csp_internal_choice  *choice = vchoice;
    if (initial == csp->tau) {
        csp_id_set_builder_merge(builder, &choice->ps);
    }
}

static void
csp_internal_choice_free(struct csp *csp, void *vchoice)
{
    struct csp_internal_choice  *choice = vchoice;
    csp_id_set_done(&choice->ps);
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
    csp_id_set_fill_double(&choice->ps, a, b);
    id = csp_internal_choice_id(&choice->ps);
    csp_process_init(csp, id, choice, &csp_internal_choice_iface);
    return id;
}

csp_id
csp_replicated_internal_choice(struct csp *csp, const struct csp_id_set *ps)
{
    csp_id  id = csp_internal_choice_id(ps);
    struct csp_internal_choice  *choice = csp_internal_choice_new();
    csp_id_set_clone(&choice->ps, ps);
    csp_process_init(csp, id, choice, &csp_internal_choice_iface);
    return id;
}
