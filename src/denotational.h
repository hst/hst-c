/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_DENOTATIONAL_H
#define HST_DENOTATIONAL_H

#include <stdbool.h>
#include <stdlib.h>

#include "basics.h"

/*------------------------------------------------------------------------------
 * Denotational semantics
 */

/* We preallocate a certain number of entries in the csp_trace struct itself, to
 * minimize malloc overhead for small traces.  We automatically calculate a
 * number that makes the size of the csp_trace itself a nice multiple of
 * sizeof(void *).  On 64-bit platforms this should currently evaluate to 13. */
#define CSP_TRACE_INTERNAL_SIZE                       \
    (((sizeof(void *) * 16) /* target overall size */ \
      - sizeof(csp_id *)    /* ids */                 \
      - sizeof(size_t)      /* count */               \
      - sizeof(size_t)      /* allocated_count */     \
      ) /                                             \
     sizeof(csp_id))

/* A sequence of events. */
struct csp_trace {
    csp_id *events;
    size_t count;
    size_t allocated_count;
    csp_id internal[CSP_TRACE_INTERNAL_SIZE];
};

void
csp_trace_init(struct csp_trace *set);

void
csp_trace_done(struct csp_trace *set);

/* Ensure that the traces's `events` field has enough allocated space to hold
 * `count` events.  The trace's `count` will be set to `count` after this
 * returns; you must then fill in the `events` field with the actual events. */
void
csp_trace_ensure_size(struct csp_trace *trace, size_t count);

bool
csp_trace_eq(const struct csp_trace *trace1, const struct csp_trace *trace2);

#endif /* HST_DENOTATIONAL_H */
