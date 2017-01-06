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

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process_iface {
    void (*initials)(struct csp *csp, struct csp_process *process,
                     struct csp_event_visitor *visitor);

    void (*afters)(struct csp *csp, struct csp_process *process, csp_id initial,
                   struct csp_id_set *set);

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
csp_process_build_afters(struct csp *csp, struct csp_process *process,
                         csp_id initial, struct csp_id_set *set);

#endif /* HST_PROCESS_H */
