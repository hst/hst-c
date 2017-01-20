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

/*------------------------------------------------------------------------------
 * Traces
 */

struct csp_trace *
csp_trace_new(const struct csp_event *event, struct csp_process *process,
              struct csp_trace *prev)
{
    struct csp_trace *trace = malloc(sizeof(struct csp_trace));
    assert(trace != NULL);
    trace->event = event;
    trace->process = process;
    trace->length = 1 + ((prev == NULL) ? 0 : prev->length);
    trace->prev = prev;
    return trace;
}

void
csp_trace_free(struct csp_trace *trace)
{
    free(trace);
}

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2)
{
    if (trace1 == NULL || trace2 == NULL) {
        return trace1 == trace2;
    }
    if (trace1->event != trace2->event) {
        return false;
    }
    return csp_trace_eq(trace1->prev, trace2->prev);
}

static void
csp_trace_print_one(struct csp *csp, const struct csp_trace *trace,
                    struct csp_name_visitor *visitor)
{
    if (trace != NULL) {
        csp_trace_print_one(csp, trace->prev, visitor);
        if (trace->prev != NULL) {
            csp_name_visitor_call(csp, visitor, ",");
        }
        csp_name_visitor_call(csp, visitor, csp_event_name(trace->event));
    }
}

void
csp_trace_print(struct csp *csp, const struct csp_trace *trace,
                struct csp_name_visitor *visitor)
{
    csp_name_visitor_call(csp, visitor, "⟨");
    csp_trace_print_one(csp, trace, visitor);
    csp_name_visitor_call(csp, visitor, "⟩");
}
