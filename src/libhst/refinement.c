/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "hst.h"

#if defined(REFINEMENT_DEBUG)
#include <stdio.h>
#define XDEBUG(...) fprintf(stderr, __VA_ARGS__)
#define XDEBUG_PROCESS_SET(set)                    \
    do {                                           \
        bool __first = true;                       \
        size_t __i;                                \
        XDEBUG("{");                               \
        for (__i = 0; __i < (set)->count; __i++) { \
            if (__first) {                         \
                __first = false;                   \
            } else {                               \
                XDEBUG(",");                       \
            }                                      \
            XDEBUG(CSP_ID_FMT, (set)->ids[__i]);   \
        }                                          \
        XDEBUG("}");                               \
    } while (0)
#else
#define XDEBUG(...)             /* do nothing */
#define XDEBUG_PROCESS_SET(set) /* do nothing */
#endif

#define DEBUG(...)           \
    do {                     \
        XDEBUG(__VA_ARGS__); \
        XDEBUG("\n");        \
    } while (0)
#define DEBUG_PROCESS_SET(set)   \
    do {                         \
        XDEBUG_PROCESS_SET(set); \
        XDEBUG("\n");            \
    } while (0)

void
csp_process_find_closure(struct csp *csp, csp_id event,
                         struct csp_id_set *processes)
{
    struct csp_id_set_builder queue;
    struct csp_id_set_builder seen;
    struct csp_id_set_builder result;
    csp_id_set_builder_init(&queue);
    csp_id_set_builder_init(&seen);
    csp_id_set_builder_init(&result);

    csp_id_set_builder_merge(&queue, processes);
    csp_id_set_build(processes, &queue);
    while (processes->count > 0) {
        size_t i;
        for (i = 0; i < processes->count; i++) {
            csp_id process = processes->ids[i];
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

csp_id
csp_process_prenormalize(struct csp *csp, struct csp_normalized_lts *lts,
                         csp_id process)
{
    csp_id node;
    struct csp_id_set closure;
    struct csp_id_set pending;
    struct csp_id_set initials;
    struct csp_id_set_builder builder;
    struct csp_id_set_builder pending_builder;
    csp_id_set_init(&closure);
    csp_id_set_init(&pending);
    csp_id_set_init(&initials);
    csp_id_set_builder_init(&builder);
    csp_id_set_builder_init(&pending_builder);

    DEBUG("Prenormalize " CSP_ID_FMT, process);

    /* First find the τ-closure of the initial process. */
    csp_id_set_fill_single(&closure, process);
    csp_process_find_closure(csp, csp->tau, &closure);
    XDEBUG("τ closure is ");
    DEBUG_PROCESS_SET(&closure);

    /* Start processing with the initial process. */
    if (csp_normalized_lts_add_node(lts, &closure, &node)) {
        DEBUG("Enqueue normalized node " CSP_ID_FMT, node);
        csp_id_set_fill_single(&pending, node);
    }

    /* Keep processing normalized LTS nodes until we run out. */
    while (pending.count > 0) {
        size_t i;
        for (i = 0; i < pending.count; i++) {
            size_t j;
            csp_id current = pending.ids[i];
            const struct csp_id_set *current_processes =
                    csp_normalized_lts_get_node_processes(lts, current);
            DEBUG("Process normalized node " CSP_ID_FMT, current);
            XDEBUG("representing processes ");
            DEBUG_PROCESS_SET(current_processes);

            /* Find all of the non-τ events that any of the current processes
             * can perform. */
            for (j = 0; j < current_processes->count; j++) {
                csp_id process = current_processes->ids[j];
                DEBUG("Get initials of " CSP_ID_FMT, process);
                csp_process_build_initials(csp, process, &builder);
            }
            csp_id_set_builder_remove(&builder, csp->tau);
            csp_id_set_build(&initials, &builder);
            XDEBUG("Merged initials are ");
            DEBUG_PROCESS_SET(&initials);

            /* For each of those events, union together the `afters` of that
             * event for all of the processes in the current set. */
            for (j = 0; j < initials.count; j++) {
                size_t k;
                csp_id initial = initials.ids[j];
                csp_id after;
                DEBUG("Get afters for " CSP_ID_FMT, initial);
                for (k = 0; k < current_processes->count; k++) {
                    csp_id process = current_processes->ids[k];
                    DEBUG("Get afters of " CSP_ID_FMT " for " CSP_ID_FMT,
                          process, initial);
                    csp_process_build_afters(csp, process, initial, &builder);
                }

                /* Calculate the τ-closure of that set. */
                csp_id_set_build(&closure, &builder);
                XDEBUG("Merged afters are ");
                DEBUG_PROCESS_SET(&closure);
                csp_process_find_closure(csp, csp->tau, &closure);
                XDEBUG("τ closure is ");
                DEBUG_PROCESS_SET(&closure);

                /* Make sure that we have a normalized node for the result. */
                if (csp_normalized_lts_add_node(lts, &closure, &after)) {
                    /* If this is a new normalized node, add it to the pending
                     * set so that we process it in the next iteration of the
                     * outer loop. */
                    DEBUG("Enqueue normalized node " CSP_ID_FMT, after);
                    csp_id_set_builder_add(&pending_builder, after);
                }

                /* Add an edge to the normalized LTS. */
                csp_normalized_lts_add_edge(lts, current, initial, after);
            }
        }

        /* Finalize the pending set for the next round. */
        csp_id_set_build(&pending, &pending_builder);
    }

    csp_id_set_done(&closure);
    csp_id_set_done(&pending);
    csp_id_set_done(&initials);
    csp_id_set_builder_done(&builder);
    csp_id_set_builder_done(&pending_builder);
    return node;
}
