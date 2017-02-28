/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "denotational.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/container_of/container_of.h"
#include "environment.h"
#include "event.h"
#include "macros.h"
#include "normalization.h"
#include "operators.h"
#include "process.h"
#include "refinement.h"

/*------------------------------------------------------------------------------
 * Traces
 */

#define CSP_EMPTY_TRACE_HASH UINT64_C(0x0e40beb4b610bc78) /* random */

struct csp_trace
csp_trace_init(const struct csp_event *event, struct csp_trace *prev)
{
    struct csp_trace trace = {event, prev, prev->hash ^ csp_event_id(event)};
    return trace;
}

struct csp_trace
csp_trace_init_empty(void)
{
    struct csp_trace trace = {NULL, NULL, CSP_EMPTY_TRACE_HASH};
    return trace;
}

struct csp_trace *
csp_trace_new(const struct csp_event *event, struct csp_trace *prev)
{
    struct csp_trace *trace = malloc(sizeof(struct csp_trace));
    assert(trace != NULL);
    *trace = csp_trace_init(event, prev);
    return trace;
}

struct csp_trace *
csp_trace_new_empty(void)
{
    struct csp_trace *trace = malloc(sizeof(struct csp_trace));
    assert(trace != NULL);
    *trace = csp_trace_init_empty();
    return trace;
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
    struct csp_trace_visitor visitor;
    struct csp_name_visitor *wrapped;
    bool first;
};

static void
csp_trace_print_visit(struct csp *csp, struct csp_trace_visitor *visitor,
                      const struct csp_trace *trace)
{
    struct csp_trace_print *self =
            container_of(visitor, struct csp_trace_print, visitor);
    if (csp_trace_empty(trace)) {
        return;
    }
    if (self->first) {
        self->first = false;
    } else {
        csp_name_visitor_call(csp, self->wrapped, ",");
    }
    csp_name_visitor_call(csp, self->wrapped, csp_event_name(trace->event));
}

void
csp_trace_print(struct csp *csp, const struct csp_trace *trace,
                struct csp_name_visitor *visitor)
{
    struct csp_trace_print self = {{csp_trace_print_visit}, visitor, true};
    csp_name_visitor_call(csp, visitor, "⟨");
    csp_trace_visit_prefixes(csp, trace, &self.visitor);
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
csp_trace_visitor_call(struct csp *csp, struct csp_trace_visitor *visitor,
                       const struct csp_trace *trace)
{
    visitor->visit(csp, visitor, trace);
}

static void
csp_print_traces_visit(struct csp *csp, struct csp_trace_visitor *visitor,
                       const struct csp_trace *trace)
{
    struct csp_print_traces *self =
            container_of(visitor, struct csp_print_traces, visitor);
    csp_trace_print(csp, trace, self->wrapped);
    csp_name_visitor_call(csp, self->wrapped, "\n");
}

struct csp_print_traces
csp_print_traces(struct csp_name_visitor *wrapped)
{
    struct csp_print_traces self = {{csp_print_traces_visit}, wrapped};
    return self;
}

static void
csp_trace_visit_one_prefix(struct csp *csp, const struct csp_trace *trace,
                           struct csp_trace_visitor *visitor)
{
    if (!csp_trace_empty(trace)) {
        csp_trace_visit_one_prefix(csp, trace->prev, visitor);
    }
    csp_trace_visitor_call(csp, visitor, trace);
}

void
csp_trace_visit_prefixes(struct csp *csp, const struct csp_trace *trace,
                         struct csp_trace_visitor *visitor)
{
    csp_trace_visit_one_prefix(csp, trace, visitor);
}

/*------------------------------------------------------------------------------
 * Finite traces
 */

/* The prenormalization code can do most of the work for us; it will give us a
 * bunch of subprocesses with at most one outgoing transition for any event.  We
 * then just have to walk through its edges. */

struct csp_subprocess_traces {
    struct csp_edge_visitor visitor;
    struct csp_trace_visitor *wrapped;
    struct csp_subprocess_traces *prev;
    struct csp_process *process;
    struct csp_trace trace;
    bool any_afters;
};

static bool
csp_subprocess_traces_contains_process(struct csp_subprocess_traces *self,
                                       struct csp_process *process)
{
    if (self == NULL) {
        return false;
    }
    if (self->process == process) {
        return true;
    }
    return csp_subprocess_traces_contains_process(self->prev, process);
}

static void
csp_subprocess_traces_visit(struct csp *csp, struct csp_edge_visitor *visitor,
                            const struct csp_event *initial,
                            struct csp_process *after)
{
    struct csp_subprocess_traces *self =
            container_of(visitor, struct csp_subprocess_traces, visitor);
    struct csp_subprocess_traces next;

    self->any_afters = true;
    next.visitor.visit = csp_subprocess_traces_visit;
    next.wrapped = self->wrapped;
    next.prev = self;
    next.process = after;
    next.trace = csp_trace_init(initial, &self->trace);
    next.any_afters = false;

    /* If the current trace already contains `after`, then we've encountered a
     * cycle, and have encountered a maximal finite trace. */
    if (csp_subprocess_traces_contains_process(self, after)) {
        csp_trace_visitor_call(csp, self->wrapped, &next.trace);
    } else {
        csp_process_visit_transitions(csp, after, &next.visitor);
        if (!next.any_afters) {
            csp_trace_visitor_call(csp, self->wrapped, &next.trace);
        }
    }
}

void
csp_process_visit_maximal_finite_traces(struct csp *csp,
                                        struct csp_process *process,
                                        struct csp_trace_visitor *wrapped)
{
    struct csp_process *prenormalized = csp_prenormalize_process(csp, process);
    struct csp_subprocess_traces start = {
            {csp_subprocess_traces_visit}, wrapped, NULL, prenormalized,
            csp_trace_init_empty(),        false};
    csp_process_visit_transitions(csp, prenormalized, &start.visitor);
    if (!start.any_afters) {
        csp_trace_visitor_call(csp, wrapped, &start.trace);
    }
}

/*------------------------------------------------------------------------------
 * Traced process
 */

struct csp_traced_process {
    struct csp_process process;
    struct csp_process *wrapped;
    struct csp_trace trace;
};

static void
csp_traced_process_name(struct csp *csp, struct csp_process *process,
                        struct csp_name_visitor *visitor)
{
    struct csp_traced_process *traced =
            container_of(process, struct csp_traced_process, process);
    csp_trace_print(csp, &traced->trace, visitor);
    csp_name_visitor_call(csp, visitor, " ⇒ ");
    csp_process_name(csp, traced->wrapped, visitor);
}

static void
csp_traced_process_initials(struct csp *csp, struct csp_process *process,
                            struct csp_event_visitor *visitor)
{
    struct csp_traced_process *traced =
            container_of(process, struct csp_traced_process, process);
    csp_process_visit_initials(csp, traced->wrapped, visitor);
}

struct csp_traced_process_after {
    struct csp_edge_visitor visitor;
    const struct csp_trace *trace;
    struct csp_edge_visitor *wrapped;
};

static void
csp_traced_process_after_visit(struct csp *csp,
                               struct csp_edge_visitor *visitor,
                               const struct csp_event *initial,
                               struct csp_process *wrapped_after)
{
    struct csp_traced_process_after *traced =
            container_of(visitor, struct csp_traced_process_after, visitor);
    struct csp_process *after =
            csp_traced_process_new(csp, wrapped_after, traced->trace);
    csp_edge_visitor_call(csp, traced->wrapped, initial, after);
}

static struct csp_traced_process_after
csp_traced_process_after(const struct csp_trace *trace,
                         struct csp_edge_visitor *wrapped)
{
    struct csp_traced_process_after self = {
            {csp_traced_process_after_visit}, trace, wrapped};
    return self;
}

static void
csp_traced_process_afters(struct csp *csp, struct csp_process *process,
                          const struct csp_event *initial,
                          struct csp_edge_visitor *visitor)
{
    struct csp_traced_process *traced =
            container_of(process, struct csp_traced_process, process);
    struct csp_trace trace;
    struct csp_traced_process_after after;
    trace = csp_trace_init(initial, &traced->trace);
    after = csp_traced_process_after(&trace, visitor);
    csp_process_visit_afters(csp, traced->wrapped, initial, &after.visitor);
}

static void
csp_traced_process_free(struct csp *csp, struct csp_process *process)
{
    struct csp_traced_process *traced =
            container_of(process, struct csp_traced_process, process);
    free(traced);
}

static const struct csp_process_iface csp_traced_process_iface = {
        0, csp_traced_process_name, csp_traced_process_initials,
        csp_traced_process_afters, csp_traced_process_free};

static csp_id
csp_traced_process_get_id(struct csp_process *wrapped,
                          const struct csp_trace *trace)
{
    static struct csp_id_scope traced_process;
    csp_id id = csp_id_start(&traced_process);
    id = csp_id_add_process(id, wrapped);
    id = csp_id_add_id(id, trace->hash);
    return id;
}

struct csp_process *
csp_traced_process_new(struct csp *csp, struct csp_process *wrapped,
                       const struct csp_trace *trace)
{
    csp_id id = csp_traced_process_get_id(wrapped, trace);
    struct csp_traced_process *traced;
    return_if_nonnull(csp_get_process(csp, id));
    traced = malloc(sizeof(struct csp_traced_process));
    assert(traced != NULL);
    traced->process.id = id;
    traced->process.iface = &csp_traced_process_iface;
    traced->wrapped = wrapped;
    traced->trace = *trace;
    csp_register_process(csp, &traced->process);
    return &traced->process;
}

static struct csp_traced_process *
csp_traced_process_downcast(struct csp_process *process)
{
    assert(process->iface == &csp_traced_process_iface);
    return container_of(process, struct csp_traced_process, process);
}

struct csp_process *
csp_traced_process(struct csp *csp, struct csp_process *wrapped)
{
    struct csp_trace trace = csp_trace_init_empty();
    return csp_traced_process_new(csp, wrapped, &trace);
}

const struct csp_trace *
csp_traced_process_get_trace(struct csp *csp, struct csp_process *process)
{
    struct csp_traced_process *traced = csp_traced_process_downcast(process);
    return &traced->trace;
}

struct csp_process *
csp_traced_process_get_wrapped(struct csp *csp, struct csp_process *process)
{
    struct csp_traced_process *traced = csp_traced_process_downcast(process);
    return traced->wrapped;
}
