/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_REFINEMENT_H
#define HST_REFINEMENT_H

#include <stdbool.h>

#include "environment.h"
#include "process.h"

/*------------------------------------------------------------------------------
 * Refinement check process
 */

/* Creates a process that contains a (Spec, Impl) pair that needs to be visited
 * during a refinement check.  `spec` should be a normalized process. */
struct csp_process *
csp_refinement_process(struct csp *csp, struct csp_process *spec,
                       struct csp_process *impl);

/*------------------------------------------------------------------------------
 * Refinement
 */

/* Return whether Spec ⊑T Impl.  We will normalize Spec for you. */
bool
csp_check_traces_refinement(struct csp *csp, struct csp_process *spec,
                            struct csp_process *impl);

#endif /* HST_REFINEMENT_H */
