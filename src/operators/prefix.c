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
#include "event.h"
#include "macros.h"
#include "process.h"

struct csp_prefix {
    struct csp_process process;
    const struct csp_event *a;
    struct csp_process *p;
};

/* Operational semantics for a → P
 *
 * 1) ─────────────
 *     a → P -a→ P
 */

static void
csp_prefix_initials(struct csp *csp, struct csp_process *process,
                    struct csp_event_visitor *visitor)
{
    /* initials(a → P) = {a} */
    struct csp_prefix *prefix =
            container_of(process, struct csp_prefix, process);
    csp_event_visitor_call(csp, visitor, prefix->a);
}

static void
csp_prefix_afters(struct csp *csp, struct csp_process *process,
                  const struct csp_event *initial,
                  struct csp_edge_visitor *visitor)
{
    /* afters(a → P, a) = P */
    struct csp_prefix *prefix =
            container_of(process, struct csp_prefix, process);
    if (initial == prefix->a) {
        csp_edge_visitor_call(csp, visitor, initial, prefix->p);
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
csp_prefix_get_id(const struct csp_event *a, struct csp_process *p)
{
    static struct csp_id_scope prefix;
    csp_id id = csp_id_start(&prefix);
    id = csp_id_add_event(id, a);
    id = csp_id_add_process(id, p);
    return id;
}

static struct csp_process *
csp_prefix_new(struct csp *csp, const struct csp_event *a,
               struct csp_process *p)
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

struct csp_process *
csp_prefix(struct csp *csp, const struct csp_event *a, struct csp_process *p)
{
    return csp_prefix_new(csp, a, p);
}
