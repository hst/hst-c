/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "behavior.h"

#include <stdlib.h>

#include "ccan/likely/likely.h"
#include "environment.h"
#include "event.h"

void
csp_behavior_init(struct csp_behavior *behavior)
{
    csp_event_set_init(&behavior->initials);
}

void
csp_behavior_done(struct csp_behavior *behavior)
{
    csp_event_set_done(&behavior->initials);
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
    return csp_event_set_eq(&b1->initials, &b2->initials);
}

bool
csp_behavior_refines(const struct csp_behavior *spec,
                     const struct csp_behavior *impl)
{
    if (unlikely(spec->model != impl->model)) {
        return false;
    }
    return csp_event_set_subseteq(&impl->initials, &spec->initials);
}

static void
csp_process_add_traces_behavior(struct csp *csp, struct csp_process *process,
                                struct csp_behavior *behavior)
{
    struct csp_collect_events collect = csp_collect_events(&behavior->initials);
    csp_process_visit_initials(csp, process, &collect.visitor);
}

static void
csp_behavior_finish_traces(struct csp *csp, struct csp_behavior *behavior)
{
    behavior->model = CSP_TRACES;
    csp_event_set_remove(&behavior->initials, csp->tau);
    behavior->hash = csp_event_set_hash(&behavior->initials);
}

static void
csp_process_get_traces_behavior(struct csp *csp, struct csp_process *process,
                                struct csp_behavior *behavior)
{
    csp_event_set_clear(&behavior->initials);
    csp_process_add_traces_behavior(csp, process, behavior);
    csp_behavior_finish_traces(csp, behavior);
}

void
csp_process_get_behavior(struct csp *csp, struct csp_process *process,
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
                                    const struct csp_process_set *processes,
                                    struct csp_behavior *behavior)
{
    struct csp_process_set_iterator iter;
    csp_event_set_clear(&behavior->initials);
    csp_process_set_foreach (processes, &iter) {
        struct csp_process *process = csp_process_set_iterator_get(&iter);
        csp_process_add_traces_behavior(csp, process, behavior);
    }
    csp_behavior_finish_traces(csp, behavior);
}

void
csp_process_set_get_behavior(struct csp *csp,
                             const struct csp_process_set *processes,
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
