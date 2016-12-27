/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "hst.h"

struct csp_prefix {
    csp_id a;
    csp_id p;
};

/* Operational semantics for a → P
 *
 * 1) ─────────────
 *     a → P -a→ P
 */

static void
csp_prefix_initials(struct csp *csp, struct csp_id_set *set, void *vprefix)
{
    /* initials(a → P) = {a} */
    struct csp_prefix *prefix = vprefix;
    csp_id_set_add(set, prefix->a);
}

static void
csp_prefix_afters(struct csp *csp, csp_id initial, struct csp_id_set *set,
                  void *vprefix)
{
    /* afters(a → P, a) = P */
    struct csp_prefix *prefix = vprefix;
    if (initial == prefix->a) {
        csp_id_set_add(set, prefix->p);
    }
}

static csp_id
csp_prefix_get_id(struct csp *csp, const void *vinput)
{
    const struct csp_prefix *input = vinput;
    static struct csp_id_scope prefix;
    csp_id id = csp_id_start(&prefix);
    id = csp_id_add_id(id, input->a);
    id = csp_id_add_id(id, input->p);
    return id;
}

static size_t
csp_prefix_ud_size(struct csp *csp, const void *vinput)
{
    return sizeof(struct csp_prefix);
}

static void
csp_prefix_init(struct csp *csp, void *vprefix, const void *vinput)
{
    struct csp_prefix *prefix = vprefix;
    const struct csp_prefix *input = vinput;
    prefix->a = input->a;
    prefix->p = input->p;
}

static void
csp_prefix_done(struct csp *csp, void *vprefix)
{
    /* nothing to do */
}

static const struct csp_process_iface csp_prefix_iface = {
        &csp_prefix_initials, &csp_prefix_afters, &csp_prefix_get_id,
        &csp_prefix_ud_size,  &csp_prefix_init,   &csp_prefix_done};

csp_id
csp_prefix(struct csp *csp, csp_id a, csp_id p)
{
    struct csp_prefix input = {a, p};
    return csp_process_init(csp, &input, NULL, &csp_prefix_iface);
}
