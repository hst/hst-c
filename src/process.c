/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "process.h"

#include "ccan/container_of/container_of.h"
#include "basics.h"
#include "environment.h"
#include "id-set.h"
#include "macros.h"

/*------------------------------------------------------------------------------
 * Event visitors
 */

void
csp_event_visitor_call(struct csp *csp, struct csp_event_visitor *visitor,
                       csp_id event)
{
    visitor->visit(csp, visitor, event);
}

static void
csp_collect_events_visit(struct csp *csp, struct csp_event_visitor *visitor,
                         csp_id event)
{
    struct csp_collect_events *self =
            container_of(visitor, struct csp_collect_events, visitor);
    csp_id_set_add(self->set, event);
}

struct csp_collect_events
csp_collect_events(struct csp_id_set *set)
{
    struct csp_collect_events self = {{csp_collect_events_visit}, set};
    return self;
}

static void
csp_ignore_event_visit(struct csp *csp, struct csp_event_visitor *visitor,
                         csp_id event)
{
    struct csp_ignore_event *self =
            container_of(visitor, struct csp_ignore_event, visitor);
    if (event != self->event) {
        csp_event_visitor_call(csp, self->wrapped, event);
    }
}

struct csp_ignore_event
csp_ignore_event(struct csp_event_visitor *wrapped, csp_id event)
{
    struct csp_ignore_event self = {{csp_ignore_event_visit}, wrapped, event};
    return self;
}

/*------------------------------------------------------------------------------
 * Edge visitors
 */

void
csp_edge_visitor_call(struct csp *csp, struct csp_edge_visitor *visitor,
                      csp_id event, csp_id after)
{
    visitor->visit(csp, visitor, event, after);
}

static void
csp_collect_afters_visit(struct csp *csp, struct csp_edge_visitor *visitor,
                         csp_id event, csp_id after)
{
    struct csp_collect_afters *self =
            container_of(visitor, struct csp_collect_afters, visitor);
    csp_id_set_add(self->set, after);
}

struct csp_collect_afters
csp_collect_afters(struct csp_id_set *set)
{
    struct csp_collect_afters self = {{csp_collect_afters_visit}, set};
    return self;
}

/*------------------------------------------------------------------------------
 * Process visitors
 */

void
csp_process_visitor_call(struct csp *csp, struct csp_process_visitor *visitor,
                         struct csp_process *process)
{
    visitor->visit(csp, visitor, process);
}

static void
csp_collect_processes_visit(struct csp *csp,
                            struct csp_process_visitor *visitor,
                            struct csp_process *process)
{
    struct csp_collect_processes *self =
            container_of(visitor, struct csp_collect_processes, visitor);
    csp_id_set_add(self->set, process->id);
}

struct csp_collect_processes
csp_collect_processes(struct csp_id_set *set)
{
    struct csp_collect_processes self = {{csp_collect_processes_visit}, set};
    return self;
}

/*------------------------------------------------------------------------------
 * Processes
 */

void
csp_process_free(struct csp *csp, struct csp_process *process)
{
    process->iface->free(csp, process);
}

void
csp_process_visit_initials(struct csp *csp, struct csp_process *process,
                           struct csp_event_visitor *visitor)
{
    process->iface->initials(csp, process, visitor);
}

void
csp_process_visit_afters(struct csp *csp, struct csp_process *process,
                         csp_id initial, struct csp_edge_visitor *visitor)
{
    process->iface->afters(csp, process, initial, visitor);
}

struct csp_process_visit_transitions {
    struct csp_process *process;
    struct csp_edge_visitor *wrapped;
    struct csp_event_visitor visit_initial;
};

static void
csp_process_visit_transitions_visit_initial(struct csp *csp,
                                            struct csp_event_visitor *visitor,
                                            csp_id initial)
{
    struct csp_process_visit_transitions *self = container_of(
            visitor, struct csp_process_visit_transitions, visit_initial);
    csp_process_visit_afters(csp, self->process, initial, self->wrapped);
}

void
csp_process_visit_transitions(struct csp *csp, struct csp_process *process,
                              struct csp_edge_visitor *visitor)
{
    struct csp_process_visit_transitions self = {
            process, visitor, {csp_process_visit_transitions_visit_initial}};
    csp_process_visit_initials(csp, process, &self.visit_initial);
}

struct csp_process_bfs {
    struct csp_id_set seen;
    struct csp_id_set queue1;
    struct csp_id_set queue2;
    struct csp_id_set *current_queue;
    struct csp_id_set *next_queue;
    struct csp_process_visitor *wrapped;
    struct csp_edge_visitor visit_transition;
};

static void
csp_process_bfs_enqueue(struct csp *csp, struct csp_process_bfs *self,
                        csp_id process_id)
{
    if (csp_id_set_add(&self->seen, process_id)) {
        csp_id_set_add(self->next_queue, process_id);
    }
}

static void
csp_process_bfs_visit_transition(struct csp *csp,
                                 struct csp_edge_visitor *visitor,
                                 csp_id initial, csp_id after)
{
    struct csp_process_bfs *self =
            container_of(visitor, struct csp_process_bfs, visit_transition);
    csp_process_bfs_enqueue(csp, self, after);
}

static void
csp_process_bfs_visit_process(struct csp *csp, struct csp_process_bfs *self,
                              struct csp_process *process)
{
    csp_process_visitor_call(csp, self->wrapped, process);
    csp_process_visit_transitions(csp, process, &self->visit_transition);
}

static void
csp_process_bfs_init(struct csp_process_bfs *self,
                     struct csp_process_visitor *wrapped)
{
    csp_id_set_init(&self->seen);
    csp_id_set_init(&self->queue1);
    csp_id_set_init(&self->queue2);
    self->current_queue = &self->queue1;
    self->next_queue = &self->queue2;
    self->wrapped = wrapped;
    self->visit_transition.visit = csp_process_bfs_visit_transition;
}

static void
csp_process_bfs_done(struct csp_process_bfs *self)
{
    csp_id_set_done(&self->seen);
    csp_id_set_done(&self->queue1);
    csp_id_set_done(&self->queue2);
}

void
csp_process_bfs(struct csp *csp, struct csp_process *root,
                struct csp_process_visitor *visitor)
{
    struct csp_process_bfs self;
    csp_process_bfs_init(&self, visitor);
    csp_process_bfs_enqueue(csp, &self, root->id);
    while (!csp_id_set_empty(self.next_queue)) {
        struct csp_id_set_iterator iter;
        swap(self.current_queue, self.next_queue);
        csp_id_set_clear(self.next_queue);
        csp_id_set_foreach (self.current_queue, &iter) {
            csp_id process_id = csp_id_set_iterator_get(&iter);
            struct csp_process *process = csp_require_process(csp, process_id);
            csp_process_bfs_visit_process(csp, &self, process);
        }
    }
    csp_process_bfs_done(&self);
}
