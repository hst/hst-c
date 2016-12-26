/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "ccan/likely/likely.h"
#include "hst.h"

void
csp_behavior_init(struct csp_behavior *behavior)
{
    csp_id_set_init(&behavior->initials);
}

void
csp_behavior_done(struct csp_behavior *behavior)
{
    csp_id_set_done(&behavior->initials);
}

bool
csp_behavior_eq(const struct csp_behavior *b1, const struct csp_behavior *b2)
{
    if (b1->hash != b2->hash) {
        return false;
    }
    if (unlikely(b1->model != b2->model)) {
        return false;
    }
    return csp_id_set_eq(&b1->initials, &b2->initials);
}

bool
csp_behavior_refines(const struct csp_behavior *spec,
                     const struct csp_behavior *impl)
{
    if (unlikely(spec->model != impl->model)) {
        return false;
    }
    return csp_id_set_subseteq(&impl->initials, &spec->initials);
}

static void
csp_process_add_traces_behavior(struct csp *csp, csp_id process,
                                struct csp_behavior *behavior)
{
    csp_process_build_initials(csp, process, &behavior->initials);
}

static void
csp_behavior_finish_traces(struct csp *csp, struct csp_behavior *behavior)
{
    behavior->model = CSP_TRACES;
    csp_id_set_remove(&behavior->initials, csp->tau);
    behavior->hash = behavior->initials.hash;
}

static void
csp_process_get_traces_behavior(struct csp *csp, csp_id process,
                                struct csp_behavior *behavior)
{
    csp_id_set_clear(&behavior->initials);
    csp_process_add_traces_behavior(csp, process, behavior);
    csp_behavior_finish_traces(csp, behavior);
}

void
csp_process_get_behavior(struct csp *csp, csp_id process,
                         enum csp_semantic_model model,
                         struct csp_behavior *behavior)
{
    switch (model) {
        case CSP_TRACES:
            csp_process_get_traces_behavior(csp, process, behavior);
            break;
        default:
            abort();
    }
}

static void
csp_process_set_get_traces_behavior(struct csp *csp,
                                    const struct csp_id_set *processes,
                                    struct csp_behavior *behavior)
{
    struct csp_id_set_iterator iter;
    csp_id_set_clear(&behavior->initials);
    csp_id_set_foreach (processes, &iter) {
        csp_process_add_traces_behavior(csp, iter.current, behavior);
    }
    csp_behavior_finish_traces(csp, behavior);
}

void
csp_process_set_get_behavior(struct csp *csp,
                             const struct csp_id_set *processes,
                             enum csp_semantic_model model,
                             struct csp_behavior *behavior)
{
    switch (model) {
        case CSP_TRACES:
            csp_process_set_get_traces_behavior(csp, processes, behavior);
            break;
        default:
            abort();
    }
}
