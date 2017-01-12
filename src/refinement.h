/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_REFINEMENT_H
#define HST_REFINEMENT_H

#include <stdbool.h>

#include "basics.h"
#include "environment.h"

/*------------------------------------------------------------------------------
 * Refinement
 */

/* Return whether Spec ⊑T Impl.  We will normalize Spec for you. */
bool
csp_check_traces_refinement(struct csp *csp, struct csp_process *spec,
                            struct csp_process *impl);

#endif /* HST_REFINEMENT_H */
