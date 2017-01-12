/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PROCESS_H
#define HST_PROCESS_H

#include "basics.h"
#include "id-set.h"

struct csp;
struct csp_process;

/*------------------------------------------------------------------------------
 * Event visitors
 */

struct csp_event_visitor {
    void (*visit)(struct csp *csp, struct csp_event_visitor *visitor,
                  csp_id event);
};

void
csp_event_visitor_call(struct csp *csp, struct csp_event_visitor *visitor,
                       csp_id event);

struct csp_collect_events {
    struct csp_event_visitor visitor;
    struct csp_id_set *set;
};

struct csp_collect_events
csp_collect_events(struct csp_id_set *set);

struct csp_ignore_event {
    struct csp_event_visitor visitor;
    struct csp_event_visitor *wrapped;
    csp_id event;
};

struct csp_ignore_event
csp_ignore_event(struct csp_event_visitor *wrapped, csp_id event);

/*------------------------------------------------------------------------------
 * Edge visitors
 */

struct csp_edge_visitor {
    void (*visit)(struct csp *csp, struct csp_edge_visitor *visitor,
                  csp_id event, csp_id after);
};

void
csp_edge_visitor_call(struct csp *csp, struct csp_edge_visitor *visitor,
                      csp_id event, csp_id after);

struct csp_collect_afters {
    struct csp_edge_visitor visitor;
    struct csp_id_set *set;
};

struct csp_collect_afters
csp_collect_afters(struct csp_id_set *set);

/*------------------------------------------------------------------------------
 * Process visitors
 */

struct csp_process_visitor {
    void (*visit)(struct csp *csp, struct csp_process_visitor *visitor,
                  struct csp_process *process);
};

void
csp_process_visitor_call(struct csp *csp, struct csp_process_visitor *visitor,
                         struct csp_process *process);

struct csp_collect_processes {
    struct csp_process_visitor visitor;
    struct csp_id_set *set;
};

struct csp_collect_processes
csp_collect_processes(struct csp_id_set *set);

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process_iface {
    void (*initials)(struct csp *csp, struct csp_process *process,
                     struct csp_event_visitor *visitor);

    void (*afters)(struct csp *csp, struct csp_process *process, csp_id initial,
                   struct csp_edge_visitor *visitor);

    void (*free)(struct csp *csp, struct csp_process *process);
};

struct csp_process {
    csp_id id;
    const struct csp_process_iface *iface;
};

void
csp_process_free(struct csp *csp, struct csp_process *process);

void
csp_process_visit_initials(struct csp *csp, struct csp_process *process,
                           struct csp_event_visitor *visitor);

void
csp_process_visit_afters(struct csp *csp, struct csp_process *process,
                         csp_id initial, struct csp_edge_visitor *visitor);

void
csp_process_visit_transitions(struct csp *csp, struct csp_process *process,
                              struct csp_edge_visitor *visitor);

void
csp_process_bfs(struct csp *csp, struct csp_process *process,
                struct csp_process_visitor *visitor);

/*------------------------------------------------------------------------------
 * Process sets
 */

struct csp_process_set {
    struct csp_set set;
};

void
csp_process_set_init(struct csp_process_set *set);

void
csp_process_set_done(struct csp_process_set *set);

bool
csp_process_set_empty(const struct csp_process_set *set);

size_t
csp_process_set_size(const struct csp_process_set *set);

bool
csp_process_set_eq(const struct csp_process_set *set1,
                   const struct csp_process_set *set2);

void
csp_process_set_clear(struct csp_process_set *set);

/* Add a single process to a set.  Return whether the process is new (i.e., it
 * wasn't already in `set`.) */
bool
csp_process_set_add(struct csp_process_set *set, struct csp_process *process);

/* Remove a single process from a set.  Returns whether that process was in the
 * set or not. */
bool
csp_process_set_remove(struct csp_process_set *set,
                       struct csp_process *process);

/* Add the contents of an existing set to a set.  Returns true if any new
 * elements were added. */
bool
csp_process_set_union(struct csp_process_set *set,
                      const struct csp_process_set *other);

struct csp_process_set_iterator {
    struct csp_set_iterator iter;
};

void
csp_process_set_get_iterator(const struct csp_process_set *set,
                             struct csp_process_set_iterator *iter);

struct csp_process *
csp_process_set_iterator_get(const struct csp_process_set_iterator *iter);

bool
csp_process_set_iterator_done(struct csp_process_set_iterator *iter);

void
csp_process_set_iterator_advance(struct csp_process_set_iterator *iter);

#define csp_process_set_foreach(set, iter)            \
    for (csp_process_set_get_iterator((set), (iter)); \
         !csp_process_set_iterator_done((iter));      \
         csp_process_set_iterator_advance((iter)))

#endif /* HST_PROCESS_H */
