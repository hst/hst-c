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
#include "macros.h"
#include "normalization.h"
#include "process.h"

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
    if (trace == NULL) {
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

struct csp_process_has_trace {
    struct csp_trace_event_visitor visitor;
    struct csp_process_set set1;
    struct csp_process_set set2;
    struct csp_process_set *current;
    struct csp_process_set *afters;
};

static void
csp_process_has_trace_visit(struct csp *csp,
                            struct csp_trace_event_visitor *visitor,
                            const struct csp_trace *trace, size_t index)
{
    struct csp_process_has_trace *self =
            container_of(visitor, struct csp_process_has_trace, visitor);
    struct csp_process_set_iterator iter;
    struct csp_collect_afters collect;

    if (trace == NULL) {
        return;
    }

    /* We might currently be in (the τ closure of) any of the `current`
     * processes.  Find which processes we could end up in if we follow the
     * current trace event from one of them. */
    csp_find_process_closure(csp, csp->tau, self->current);
    csp_process_set_clear(self->afters);
    collect = csp_collect_afters(self->afters);
    csp_process_set_foreach (self->current, &iter) {
        struct csp_process *process = csp_process_set_iterator_get(&iter);
        csp_process_visit_afters(csp, process, trace->event, &collect.visitor);
    }

    /* This new `afters` set becomes the `current` set for the next event in the
     * trace. */
    swap(self->current, self->afters);
}

static void
csp_process_has_trace_init(struct csp_process_has_trace *self)
{
    self->visitor.visit = csp_process_has_trace_visit;
    csp_process_set_init(&self->set1);
    csp_process_set_init(&self->set2);
    self->current = &self->set1;
    self->afters = &self->set2;
}

static void
csp_process_has_trace_done(struct csp_process_has_trace *self)
{
    csp_process_set_done(&self->set1);
    csp_process_set_done(&self->set2);
}

bool
csp_process_has_trace(struct csp *csp, struct csp_process *process,
                      const struct csp_trace *trace)
{
    struct csp_process_has_trace self;
    bool result;
    csp_process_has_trace_init(&self);
    csp_process_set_add(self.current, process);
    csp_trace_visit_events(csp, trace, &self.visitor);
    result = !csp_process_set_empty(self.current);
    csp_process_has_trace_done(&self);
    return result;
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
    if (trace == NULL) {
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
