/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"

#if defined(REFINEMENT_DEBUG)
#include <stdio.h>
#define DEBUG(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while (0)
#else
#define DEBUG(...)   /* do nothing */
#endif

void
csp_process_find_closure(struct csp *csp, csp_id event,
                         struct csp_id_set *processes)
{
    struct csp_id_set_builder  queue;
    struct csp_id_set_builder  seen;
    struct csp_id_set_builder  result;
    csp_id_set_builder_init(&queue);
    csp_id_set_builder_init(&seen);
    csp_id_set_builder_init(&result);

    csp_id_set_builder_merge(&queue, processes);
    csp_id_set_build(processes, &queue);
    while (processes->count > 0) {
        size_t  i;
        for (i = 0; i < processes->count; i++) {
            csp_id  process = processes->ids[i];
            DEBUG("process " CSP_ID_FMT, process);
            /* Don't handle this process more than once. */
            if (csp_id_set_builder_add(&seen, process)) {
                DEBUG("NEW     " CSP_ID_FMT, process);
                /* Add this process to the final result. */
                csp_id_set_builder_add(&result, process);
                /* Enqueue each of the states that we can reach from `process`
                 * by following a single `event`. */
                csp_process_build_afters(csp, process, event, &queue);
            }
        }
        csp_id_set_build(processes, &queue);
    }

    csp_id_set_build(processes, &result);
    csp_id_set_builder_done(&queue);
    csp_id_set_builder_done(&seen);
    csp_id_set_builder_done(&result);
}
