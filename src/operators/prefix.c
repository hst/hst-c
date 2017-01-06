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

struct csp_prefix {
    struct csp_process process;
    csp_id a;
    csp_id p;
};

/* Operational semantics for a → P
 *
 * 1) ─────────────
 *     a → P -a→ P
 */

static void
csp_prefix_initials(struct csp *csp, struct csp_process *process,
                    struct csp_id_set *set)
{
    /* initials(a → P) = {a} */
    struct csp_prefix *prefix =
            container_of(process, struct csp_prefix, process);
    csp_id_set_add(set, prefix->a);
}

static void
csp_prefix_afters(struct csp *csp, struct csp_process *process, csp_id initial,
                  struct csp_id_set *set)
{
    /* afters(a → P, a) = P */
    struct csp_prefix *prefix =
            container_of(process, struct csp_prefix, process);
    if (initial == prefix->a) {
        csp_id_set_add(set, prefix->p);
    }
}

static void
csp_prefix_free(struct csp *csp, struct csp_process *process)
{
    struct csp_prefix *prefix =
            container_of(process, struct csp_prefix, process);
    free(prefix);
}

static const struct csp_process_iface csp_prefix_iface = {
        csp_prefix_initials, csp_prefix_afters, csp_prefix_free};

static csp_id
csp_prefix_get_id(csp_id a, csp_id p)
{
    static struct csp_id_scope prefix;
    csp_id id = csp_id_start(&prefix);
    id = csp_id_add_id(id, a);
    id = csp_id_add_id(id, p);
    return id;
}

static struct csp_process *
csp_prefix_new(struct csp *csp, csp_id a, csp_id p)
{
    csp_id id = csp_prefix_get_id(a, p);
    struct csp_prefix *prefix;
    return_if_nonnull(csp_get_process(csp, id));
    prefix = malloc(sizeof(struct csp_prefix));
    assert(prefix != NULL);
    prefix->process.id = id;
    prefix->process.iface = &csp_prefix_iface;
    prefix->a = a;
    prefix->p = p;
    csp_register_process(csp, &prefix->process);
    return &prefix->process;
}

csp_id
csp_prefix(struct csp *csp, csp_id a, csp_id p)
{
    return csp_prefix_new(csp, a, p)->id;
}
