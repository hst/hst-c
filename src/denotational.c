/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "denotational.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/container_of/container_of.h"
#include "event.h"
#include "operators.h"
#include "process.h"
#include "refinement.h"

/*------------------------------------------------------------------------------
 * Traces
 */

struct csp_trace
csp_trace_init(const struct csp_event *event, struct csp_trace *prev)
{
    struct csp_trace trace = {event, prev};
    return trace;
}

struct csp_trace
csp_trace_init_empty(void)
{
    return csp_trace_init(NULL, NULL);
}

struct csp_trace *
csp_trace_new(const struct csp_event *event, struct csp_trace *prev)
{
    struct csp_trace *trace = malloc(sizeof(struct csp_trace));
    assert(trace != NULL);
    trace->event = event;
    trace->prev = prev;
    return trace;
}

struct csp_trace *
csp_trace_new_empty(void)
{
    return csp_trace_new(NULL, NULL);
}

void
csp_trace_free(struct csp_trace *trace)
{
    free(trace);
}

void
csp_trace_free_deep(struct csp_trace *trace)
{
    if (trace != NULL) {
        struct csp_trace *prev = trace->prev;
        free(trace);
        csp_trace_free_deep(prev);
    }
}

bool
csp_trace_empty(const struct csp_trace *trace)
{
    return trace->event == NULL;
}

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2)
{
    if (csp_trace_empty(trace1) || csp_trace_empty(trace2)) {
        return csp_trace_empty(trace1) == csp_trace_empty(trace2);
    }
    if (trace1->event != trace2->event) {
        return false;
    }
    return csp_trace_eq(trace1->prev, trace2->prev);
}

struct csp_trace_print {
    struct csp_trace_event_visitor visitor;
    struct csp_name_visitor *wrapped;
};

static void
csp_trace_print_visit(struct csp *csp, struct csp_trace_event_visitor *visitor,
                      const struct csp_trace *trace, size_t index)
{
    struct csp_trace_print *self =
            container_of(visitor, struct csp_trace_print, visitor);
    if (csp_trace_empty(trace)) {
        return;
    }
    if (index > 1) {
        csp_name_visitor_call(csp, self->wrapped, ",");
    }
    csp_name_visitor_call(csp, self->wrapped, csp_event_name(trace->event));
}

void
csp_trace_print(struct csp *csp, const struct csp_trace *trace,
                struct csp_name_visitor *visitor)
{
    struct csp_trace_print self = {{csp_trace_print_visit}, visitor};
    csp_name_visitor_call(csp, visitor, "⟨");
    csp_trace_visit_events(csp, trace, &self.visitor);
    csp_name_visitor_call(csp, visitor, "⟩");
}

struct csp_process *
csp_process_from_trace(struct csp *csp, const struct csp_trace *trace)
{
    struct csp_process *current = csp->stop;
    while (!csp_trace_empty(trace)) {
        current = csp_prefix(csp, trace->event, current);
        trace = trace->prev;
    }
    return current;
}

bool
csp_process_has_trace(struct csp *csp, struct csp_process *process,
                      const struct csp_trace *trace)
{
    assert(trace != NULL);
    struct csp_process *trace_process = csp_process_from_trace(csp, trace);
    return csp_check_traces_refinement(csp, process, trace_process);
}

/*------------------------------------------------------------------------------
 * Trace element visitor
 */

void
csp_trace_event_visitor_call(struct csp *csp,
                             struct csp_trace_event_visitor *visitor,
                             const struct csp_trace *trace, size_t index)
{
    visitor->visit(csp, visitor, trace, index);
}

static size_t
csp_trace_visit_one_event(struct csp *csp, const struct csp_trace *trace,
                          struct csp_trace_event_visitor *visitor)
{
    size_t index;
    if (csp_trace_empty(trace)) {
        index = 0;
    } else {
        index = csp_trace_visit_one_event(csp, trace->prev, visitor) + 1;
    }
    csp_trace_event_visitor_call(csp, visitor, trace, index);
    return index;
}

void
csp_trace_visit_events(struct csp *csp, const struct csp_trace *trace,
                       struct csp_trace_event_visitor *visitor)
{
    csp_trace_visit_one_event(csp, trace, visitor);
}
