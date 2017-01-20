/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_NORMALIZATION_H
#define HST_NORMALIZATION_H

#include "basics.h"
#include "environment.h"
#include "equivalence.h"
#include "event.h"
#include "process.h"

/*------------------------------------------------------------------------------
 * Normalized processes
 */

/* Finds the closure of a set of processes for a particular event.  This is the
 * set of processes that can be reached from any of the initial processes by
 * only following (any number of occurrences of) that event.  The event will
 * usually be τ.
 *
 * `processes` should contain the initial processes to calculate the closure
 * for; it will be updated to contain all of the processes in the closure (which
 * must always include the initial processes). */
void
csp_find_process_closure(struct csp *csp, const struct csp_event *event,
                         struct csp_process_set *processes);

/* Creates the "prenormalization" of `process`.  A prenormalized process is one
 * that is guaranteed to have at most one outgoing transition for each event,
 * and to have no τ transitions at all. */
struct csp_process *
csp_prenormalize_process(struct csp *csp, struct csp_process *process);

/* Creates the "normalization" of a prenormalized process.  A normalized process
 * has the same restrictions as a prenormalized process, but also guarantees
 * that each distinct subprocess has a distinct behavior.  The result is a
 * process that can be used as the `Spec` of a refinement check. */
struct csp_process *
csp_normalize_process(struct csp *csp, struct csp_process *prenormalized);

/*------------------------------------------------------------------------------
 * Internals
 *
 * These functions are used internally by our normalized process implementation;
 * they're included here mainly so that we can write unit tests for it. */

/* Creates the prenormalization of a set of processes.  `processes` must be
 * τ-closed. */
struct csp_process *
csp_prenormalized_process_new(struct csp *csp,
                              const struct csp_process_set *processes);

/* Returns the set of processes that a prenormalized node represents. */
const struct csp_process_set *
csp_prenormalized_process_get_processes(struct csp_process *process);

/* Finds the subprocess of a `normalized` process that corresponds to a
 * particular `prenormalized` process. */
struct csp_process *
csp_normalized_subprocess(struct csp *csp, struct csp_process *normalized,
                          struct csp_process *prenormalized);

/* Returns the set of (original, non-normalized) processes that a normalized
 * node represents. */
void
csp_normalized_process_get_processes(struct csp *csp,
                                     struct csp_process *process,
                                     struct csp_process_set *set);

/* Returns the single `after` process for a particular `initial`, or NULL if
 * there is none.  If `process` has multiple `afters` for `initial`, the result
 * is undefined.  (We only call this for normalized processes, which are
 * guaranteed to have zero or one `after` for each event.) */
struct csp_process *
csp_process_get_single_after(struct csp *csp, struct csp_process *process,
                             const struct csp_event *initial);

/* Bisimulate all of the nodes in a prenormalized process, to find nodes that
 * have equivalent behavior.  Each prenormalized subprocess reachable from
 * `prenormalized` will have an entry in `equivalence`; the value of each entry
 * will be the fully normalized process for the equivalence class that the
 * prenormalized subprocess belongs to.  All nodes in the same equivalence class
 * will have the same normalized node. */
void
csp_calculate_bisimulation(struct csp *csp, struct csp_process *prenormalized,
                           struct csp_equivalences *equiv);

#endif /* HST_NORMALIZATION_H */
