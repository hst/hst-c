/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_BEHAVIOR_H
#define HST_BEHAVIOR_H

#include "basics.h"
#include "environment.h"
#include "event.h"
#include "process.h"

/*------------------------------------------------------------------------------
 * Process behavior
 */

enum csp_semantic_model { CSP_TRACES };

struct csp_behavior {
    enum csp_semantic_model model;
    csp_id hash;
    struct csp_event_set initials;
};

void
csp_behavior_init(struct csp_behavior *behavior);

void
csp_behavior_done(struct csp_behavior *behavior);

bool
csp_behavior_eq(const struct csp_behavior *b1, const struct csp_behavior *b2);

/* Return whether `impl` refines `spec`. */
bool
csp_behavior_refines(const struct csp_behavior *spec,
                     const struct csp_behavior *impl);

/* Fill in `behavior` with the behavior of `process` in the given semantic
 * model.  You must have already initialized `behavior`. */
void
csp_process_get_behavior(struct csp *csp, struct csp_process *process,
                         enum csp_semantic_model model,
                         struct csp_behavior *behavior);

/* Fill in `behavior` with the behavior of a set of `processes` in the given
 * semantic model.  You must have already initialized `behavior`. */
void
csp_process_set_get_behavior(struct csp *csp,
                             const struct csp_process_set *processes,
                             enum csp_semantic_model model,
                             struct csp_behavior *behavior);

#endif /* HST_BEHAVIOR_H */
