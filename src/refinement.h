/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_REFINEMENT_H
#define HST_REFINEMENT_H

#include <stdbool.h>

#include "basics.h"
#include "environment.h"
#include "id-set.h"
#include "normalized-lts.h"

/*------------------------------------------------------------------------------
 * Refinement
 */

/* Finds the closure of a set of initial processes for a particular event.  This
 * is the set of processes that can be reached from any of the initial processes
 * by only following (any number of occurrences of) that event.  The event will
 * usually be τ.
 *
 * `processes` should contain the initial processes to calculate the closure
 * for; it will be updated to contain all of the processes in the closure (which
 * must always include the initial processes). */
void
csp_process_find_closure(struct csp *csp, csp_id event,
                         struct csp_id_set *processes);

/* Prenormalizes a process, adding it to a normalized LTS.  Returns the ID of
 * the normalized LTS node representing the process. */
csp_id
csp_process_prenormalize(struct csp *csp, struct csp_normalized_lts *lts,
                         csp_id process);

/* Return whether Spec ⊑T Impl.  You must have already normalized Spec into
 * `lts`.  Use csp_process_check_traces_refinement for a simpler version if you
 * only want to perform one refinement check. */
bool
csp_check_traces_refinement(struct csp *csp, struct csp_normalized_lts *lts,
                            csp_id normalized_spec, csp_id impl);

bool
csp_process_check_traces_refinement(struct csp *csp, csp_id spec, csp_id impl);

#endif /* HST_REFINEMENT_H */
