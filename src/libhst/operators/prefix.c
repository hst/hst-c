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
csp_prefix_id(csp_id event, csp_id after)
{
    csp_id  id = csp_id_start(&prefix);
    id = csp_id_add_id(id, event);
    id = csp_id_add_id(id, after);
    return id;
}

struct csp_prefix {
    csp_id  event;
    csp_id  after;
};

static struct csp_prefix *
csp_prefix_new(csp_id event, csp_id after)
{
    struct csp_prefix  *prefix = malloc(sizeof(struct csp_prefix));
    assert(prefix != NULL);
    prefix->event = event;
    prefix->after = after;
    return prefix;
}

static void
csp_prefix_initials(struct csp *csp, struct csp_id_set_builder *builder,
                    void *vprefix)
{
    struct csp_prefix  *prefix = vprefix;
    csp_id_set_builder_add(builder, prefix->event);
}

static void
csp_prefix_afters(struct csp *csp, csp_id initial,
                  struct csp_id_set_builder *builder, void *vprefix)
{
    struct csp_prefix  *prefix = vprefix;
    if (initial == prefix->event) {
        csp_id_set_builder_add(builder, prefix->after);
    }
}

static void
csp_prefix_free(struct csp *csp, void *vprefix)
{
    struct csp_prefix  *prefix = vprefix;
    csp_process_deref(csp, prefix->after);
    free(prefix);
}

const struct csp_process_iface  csp_prefix_iface = {
    &csp_prefix_initials,
    &csp_prefix_afters,
    &csp_prefix_free
};

csp_id
csp_prefix(struct csp *csp, csp_id event, csp_id after)
{
    csp_id  id = csp_prefix_id(event, after);
    struct csp_prefix  *prefix = csp_prefix_new(event, after);
    csp_process_init(csp, id, prefix, &csp_prefix_iface);
    return id;
}
