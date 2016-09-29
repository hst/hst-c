/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "hst.h"

static struct csp_id_scope  prefix;

static csp_id
csp_prefix_id(csp_id a, csp_id p)
{
    csp_id  id = csp_id_start(&prefix);
    id = csp_id_add_id(id, a);
    id = csp_id_add_id(id, p);
    return id;
}

struct csp_prefix {
    csp_id  a;
    csp_id  p;
};

static struct csp_prefix *
csp_prefix_new(csp_id a, csp_id p)
{
    struct csp_prefix  *prefix = malloc(sizeof(struct csp_prefix));
    assert(prefix != NULL);
    prefix->a = a;
    prefix->p = p;
    return prefix;
}

/* Operational semantics for a → P
 *
 * 1) ─────────────
 *     a → P -a→ P
 */

static void
csp_prefix_initials(struct csp *csp, struct csp_id_set_builder *builder,
                    void *vprefix)
{
    /* initials(a → P) = {a} */
    struct csp_prefix  *prefix = vprefix;
    csp_id_set_builder_add(builder, prefix->a);
}

static void
csp_prefix_afters(struct csp *csp, csp_id initial,
                  struct csp_id_set_builder *builder, void *vprefix)
{
    /* afters(a → P, a) = P */
    struct csp_prefix  *prefix = vprefix;
    if (initial == prefix->a) {
        csp_id_set_builder_add(builder, csp_process_ref(csp, prefix->p));
    }
}

static void
csp_prefix_free(struct csp *csp, void *vprefix)
{
    struct csp_prefix  *prefix = vprefix;
    csp_process_deref(csp, prefix->p);
    free(prefix);
}

const struct csp_process_iface  csp_prefix_iface = {
    &csp_prefix_initials,
    &csp_prefix_afters,
    &csp_prefix_free
};

csp_id
csp_prefix(struct csp *csp, csp_id a, csp_id p)
{
    csp_id  id = csp_prefix_id(a, p);
    struct csp_prefix  *prefix = csp_prefix_new(a, p);
    csp_process_init(csp, id, prefix, &csp_prefix_iface);
    return id;
}
