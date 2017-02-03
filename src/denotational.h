/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_DENOTATIONAL_H
#define HST_DENOTATIONAL_H

#include <stdbool.h>
#include <stdlib.h>

#include "environment.h"
#include "event.h"
#include "process.h"

/*------------------------------------------------------------------------------
 * Denotational semantics
 */

/* A sequence of events.  To make them easier to construct, we represent a trace
 * as a reversed linked list, and we include a reference to the process that you
 * end up in after following the trace.
 *
 * An empty trace has `event == NULL`, and must also have `prev == NULL`.
 * */
struct csp_trace {
    const struct csp_event *event;
    struct csp_trace *prev;
};

struct csp_trace
csp_trace_init(const struct csp_event *event, struct csp_trace *prev);

struct csp_trace
csp_trace_init_empty(void);

struct csp_trace *
csp_trace_new(const struct csp_event *event, struct csp_trace *prev);

struct csp_trace *
csp_trace_new_empty(void);

/* Frees (only) `trace`. */
void
csp_trace_free(struct csp_trace *trace);

bool
csp_trace_empty(const struct csp_trace *trace);

/* Frees `trace` and all of its predecessors.  Don't call this if any of the
 * predecessors are shared with other traces! */
void
csp_trace_free_deep(struct csp_trace *trace);

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2);

void
csp_trace_print(struct csp *csp, const struct csp_trace *trace,
                struct csp_name_visitor *visitor);

/* Returns the process that has only `trace` as its maximal trace. */
struct csp_process *
csp_process_from_trace(struct csp *csp, const struct csp_trace *trace);

bool
csp_process_has_trace(struct csp *csp, struct csp_process *process,
                      const struct csp_trace *trace);

/*------------------------------------------------------------------------------
 * Trace visitor
 */

struct csp_trace_visitor {
    void (*visit)(struct csp *csp, struct csp_trace_visitor *visitor,
                  const struct csp_trace *trace);
};

void
csp_trace_visitor_call(struct csp *csp, struct csp_trace_visitor *visitor,
                       const struct csp_trace *trace);

/* Prints out each trace on a separate line. */
struct csp_print_traces {
    struct csp_trace_visitor visitor;
    struct csp_name_visitor *wrapped;
};

struct csp_print_traces
csp_print_traces(struct csp_name_visitor *wrapped);

/* Calls `visitor` for each prefix of `trace`, shortest first.  One use of this
 * function is to iterate through the events of `trace` in order. */
void
csp_trace_visit_prefixes(struct csp *csp, const struct csp_trace *trace,
                         struct csp_trace_visitor *visitor);

/* Calls `visitor` for each finite trace of `process`. */
void
csp_process_visit_maximal_finite_traces(struct csp *csp,
                                        struct csp_process *process,
                                        struct csp_trace_visitor *visitor);

#endif /* HST_DENOTATIONAL_H */
