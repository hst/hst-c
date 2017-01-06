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
