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
 * end up in after following the trace. */
struct csp_trace {
    const struct csp_event *event;
    struct csp_process *process;
    size_t length;
    struct csp_trace *prev;
};

struct csp_trace *
csp_trace_new(const struct csp_event *event, struct csp_process *process,
              struct csp_trace *prev);

void
csp_trace_free(struct csp_trace *trace);

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2);

void
csp_trace_print(struct csp *csp, const struct csp_trace *trace,
                struct csp_name_visitor *visitor);

bool
csp_process_has_trace(struct csp *csp, struct csp_process *process,
                      const struct csp_trace *trace);

/*------------------------------------------------------------------------------
 * Trace element visitor
 */

/* You almost certainly want to iterate through the events of a trace in the
 * "correct" order, which isn't trivial since we store them in reverse order.
 * This helper takes care of reversing the reversal for you. */

struct csp_trace_event_visitor {
    /* This will always be called first with trace == NULL and index == 0 */
    void (*visit)(struct csp *csp, struct csp_trace_event_visitor *visitor,
                  const struct csp_trace *trace, size_t index);
};

void
csp_trace_event_visitor_call(struct csp *csp,
                             struct csp_trace_event_visitor *visitor,
                             const struct csp_trace *trace, size_t index);

void
csp_trace_visit_events(struct csp *csp, const struct csp_trace *trace,
                       struct csp_trace_event_visitor *visitor);

#endif /* HST_DENOTATIONAL_H */
