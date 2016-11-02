/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <stdlib.h>

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
    if (b1->model != b2->model) {
        return false;
    }
    return csp_id_set_eq(&b1->initials, &b2->initials);
}

static void
csp_process_add_traces_behavior(struct csp *csp, csp_id process,
                                struct csp_behavior *behavior,
                                struct csp_id_set_builder *builder)
{
    csp_process_build_initials(csp, process, builder);
    behavior->hash = behavior->initials.hash;
}

static void
csp_behavior_finish_traces(struct csp_behavior *behavior,
                           struct csp_id_set_builder *builder)
{
    behavior->model = CSP_TRACES;
    csp_id_set_build(&behavior->initials, builder);
}

static void
csp_process_get_traces_behavior(struct csp *csp, csp_id process,
                                struct csp_behavior *behavior)
{
    struct csp_id_set_builder builder;
    csp_id_set_builder_init(&builder);
    csp_process_add_traces_behavior(csp, process, behavior, &builder);
    csp_behavior_finish_traces(behavior, &builder);
    csp_id_set_builder_done(&builder);
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
    size_t i;
    struct csp_id_set_builder builder;
    csp_id_set_builder_init(&builder);
    for (i = 0; i < processes->count; i++) {
        csp_process_add_traces_behavior(csp, processes->ids[i], behavior,
                                        &builder);
    }
    csp_id_set_builder_remove(&builder, csp->tau);
    csp_behavior_finish_traces(behavior, &builder);
    csp_id_set_builder_done(&builder);
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
