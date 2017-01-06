/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "denotational.h"

#include "csp0.h"
#include "event.h"
#include "environment.h"
#include "test-case-harness.h"

static csp_id
csp_get_event_id(const char *event_name)
{
    return csp_event_id(csp_event_get(event_name));
}

#define CSP_TRACE_FIRST_ALLOCATION_COUNT 32

TEST_CASE_GROUP("traces");

TEST_CASE("can create empty trace via factory")
{
    struct csp *csp;
    struct csp_trace *trace;
    check_alloc(csp, csp_new());
    trace = csp_trace_factory_create(csp, trace());
    check(trace->count == 0);
    csp_free(csp);
}

TEST_CASE("can create 1-element trace via factory")
{
    struct csp *csp;
    struct csp_trace *trace;
    check_alloc(csp, csp_new());
    trace = csp_trace_factory_create(csp, trace("a"));
    check(trace->count == 1);
    check_id_eq(trace->events[0], csp_get_event_id("a"));
    csp_free(csp);
}

TEST_CASE("can create 5-element trace via factory")
{
    struct csp *csp;
    struct csp_trace *trace;
    check_alloc(csp, csp_new());
    trace = csp_trace_factory_create(csp, trace("a", "b", "c", "d", "e"));
    check(trace->count == 5);
    check_id_eq(trace->events[0], csp_get_event_id("a"));
    check_id_eq(trace->events[1], csp_get_event_id("b"));
    check_id_eq(trace->events[2], csp_get_event_id("c"));
    check_id_eq(trace->events[3], csp_get_event_id("d"));
    check_id_eq(trace->events[4], csp_get_event_id("e"));
    csp_free(csp);
}

TEST_CASE("can spill over into allocated storage")
{
    struct csp_trace trace;
    csp_trace_init(&trace);
    /* Resize the trace with too many elements to fit into the preallocated
     * internal storage, but few enough to fit into the default-sized
     * heap-allocated buffer. */
    csp_trace_ensure_size(&trace, CSP_TRACE_INTERNAL_SIZE + 1);
    /* Verify that we're not using the internal storage anymore. */
    check(trace.events != trace.internal);
    check(trace.allocated_count == CSP_TRACE_FIRST_ALLOCATION_COUNT);
    csp_trace_done(&trace);
}

TEST_CASE("can spill over into large allocated storage")
{
    struct csp_trace trace;
    csp_trace_init(&trace);
    /* Resize the trace with too many elements to fit into the preallocated
     * internal storage, and too many too fit into the default-sized
     * heap-allocated buffer. */
    csp_trace_ensure_size(&trace, CSP_TRACE_FIRST_ALLOCATION_COUNT + 1);
    /* Verify that we're not using the internal storage anymore. */
    check(trace.events != trace.internal);
    check(trace.allocated_count == CSP_TRACE_FIRST_ALLOCATION_COUNT * 2);
    csp_trace_done(&trace);
}

TEST_CASE("can reallocate allocated storage")
{
    struct csp_trace trace;
    csp_trace_init(&trace);
    /* Resize the trace with one too many elements to fit into the preallocated
     * internal storage, causing an initial allocation. */
    csp_trace_ensure_size(&trace, CSP_TRACE_INTERNAL_SIZE + 1);
    /* Then resize the trace again, to cause us to reallocate the heap-allocated
     * storage. */
    csp_trace_ensure_size(&trace, CSP_TRACE_FIRST_ALLOCATION_COUNT + 1);
    /* Verify that we're not using the internal storage anymore. */
    check(trace.events != trace.internal);
    check(trace.allocated_count == CSP_TRACE_FIRST_ALLOCATION_COUNT * 2);
    csp_trace_done(&trace);
}
