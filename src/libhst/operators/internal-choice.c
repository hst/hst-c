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

struct csp_internal_choice {
    struct csp_id_set  ps;
};

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

static csp_id
csp_internal_choice_get_id(struct csp *csp, const void *vps)
{
    const struct csp_id_set  *ps = vps;
    static struct csp_id_scope  internal_choice;
    csp_id  id = csp_id_start(&internal_choice);
    id = csp_id_add_id_set(id, ps);
    return id;
}

static size_t
csp_internal_choice_ud_size(struct csp *csp, const void *temp_ud)
{
    return sizeof(struct csp_internal_choice);
}

static void
csp_internal_choice_init(struct csp *csp, void *vchoice, const void *vps)
{
    struct csp_internal_choice  *choice = vchoice;
    const struct csp_id_set  *ps = vps;
    csp_id_set_init(&choice->ps);
    csp_id_set_clone(&choice->ps, ps);
}

static void
csp_internal_choice_done(struct csp *csp, void *vchoice)
{
    struct csp_internal_choice  *choice = vchoice;
    csp_id_set_done(&choice->ps);
}

const struct csp_process_iface  csp_internal_choice_iface = {
    &csp_internal_choice_initials,
    &csp_internal_choice_afters,
    &csp_internal_choice_get_id,
    &csp_internal_choice_ud_size,
    &csp_internal_choice_init,
    &csp_internal_choice_done
};

csp_id
csp_internal_choice(struct csp *csp, csp_id a, csp_id b)
{
    csp_id  id;
    struct csp_id_set  ps;
    csp_id_set_init(&ps);
    csp_id_set_fill_double(&ps, a, b);
    id = csp_process_init(csp, &ps, &csp_internal_choice_iface);
    csp_id_set_done(&ps);
    return id;
}

csp_id
csp_replicated_internal_choice(struct csp *csp, const struct csp_id_set *ps)
{
    return csp_process_init(csp, ps, &csp_internal_choice_iface);
}
