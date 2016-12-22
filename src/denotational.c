/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/likely/likely.h"
#include "hst.h"

/*------------------------------------------------------------------------------
 * Traces
 */

void
csp_trace_init(struct csp_trace *trace)
{
    trace->events = trace->internal;
    trace->allocated_count = CSP_TRACE_INTERNAL_SIZE;
    trace->count = 0;
}

void
csp_trace_done(struct csp_trace *trace)
{
    if (trace->events != trace->internal) {
        free(trace->events);
    }
}

#define CSP_TRACE_FIRST_ALLOCATION_COUNT 32

void
csp_trace_ensure_size(struct csp_trace *trace, size_t count)
{
    trace->count = count;
    if (unlikely(count > trace->allocated_count)) {
        if (trace->events == trace->internal) {
            size_t new_count = CSP_TRACE_FIRST_ALLOCATION_COUNT;
            while (count > new_count) {
                new_count *= 2;
            }
            trace->events = malloc(new_count * sizeof(csp_id));
            assert(trace->events != NULL);
            trace->allocated_count = new_count;
        } else {
            /* Whenever we reallocate, at least double the size of the existing
             * trace. */
            csp_id *new_events;
            size_t new_count = trace->allocated_count;
            do {
                new_count *= 2;
            } while (count > new_count);
            new_events = realloc(trace->events, new_count * sizeof(csp_id));
            assert(new_events != NULL);
            trace->events = new_events;
            trace->allocated_count = new_count;
        }
    }
}

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2)
{
    if (trace1->count != trace2->count) {
        return false;
    }
    return memcmp(trace1->events, trace2->events,
                  trace1->count * sizeof(csp_id)) == 0;
}
