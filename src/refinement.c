/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "refinement.h"

#include "macros.h"

#if defined(REFINEMENT_DEBUG)
#include <stdio.h>
#define XDEBUG(...) fprintf(stderr, __VA_ARGS__)
#define XDEBUG_EVENT_SET(set)                                       \
    do {                                                            \
        bool __first = true;                                        \
        size_t __i;                                                 \
        XDEBUG("{");                                                \
        for (__i = 0; __i < (set)->count; __i++) {                  \
            if (__first) {                                          \
                __first = false;                                    \
            } else {                                                \
                XDEBUG(",");                                        \
            }                                                       \
            XDEBUG("%s", csp_get_event_name(csp, (set)->ids[__i])); \
        }                                                           \
        XDEBUG("}");                                                \
    } while (0)
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
#define XDEBUG_EVENT_SET(set)   /* do nothing */
#define XDEBUG_PROCESS_SET(set) /* do nothing */
#endif

#define DEBUG(...)           \
    do {                     \
        XDEBUG(__VA_ARGS__); \
        XDEBUG("\n");        \
    } while (0)
#define DEBUG_EVENT_SET(set)   \
    do {                       \
        XDEBUG_EVENT_SET(set); \
        XDEBUG("\n");          \
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
    bool another_round_needed = true;
    struct csp_id_set queue1;
    struct csp_id_set queue2;
    struct csp_id_set *current_queue = &queue1;
    struct csp_id_set *next_queue = &queue2;
    csp_id_set_init(&queue1);
    csp_id_set_init(&queue2);
    csp_id_set_union(current_queue, processes);
    XDEBUG("=== closure of ");
    DEBUG_PROCESS_SET(processes);
    while (another_round_needed) {
        struct csp_id_set_iterator i;
        DEBUG("--- start closure iteration %p", current_queue);
        csp_id_set_clear(next_queue);
        csp_id_set_foreach (current_queue, &i) {
            csp_id process = csp_id_set_iterator_get(&i);
            DEBUG("process " CSP_ID_FMT, process);
            /* Enqueue each of the states that we can reach from `process` by
             * following a single `event`. */
            csp_build_process_afters(csp, process, event, next_queue);
        }
        another_round_needed = csp_id_set_union(processes, next_queue);
        swap(current_queue, next_queue);
    }
    csp_id_set_done(&queue1);
    csp_id_set_done(&queue2);
}

csp_id
csp_process_prenormalize(struct csp *csp, struct csp_normalized_lts *lts,
                         csp_id process)
{
    csp_id node;
    struct csp_id_set closure;
    struct csp_id_set pending1;
    struct csp_id_set pending2;
    struct csp_id_set *current_pending = &pending1;
    struct csp_id_set *next_pending = &pending2;
    struct csp_id_set initials;
    csp_id_set_init(&closure);
    csp_id_set_init(&pending1);
    csp_id_set_init(&pending2);
    csp_id_set_init(&initials);

    DEBUG("Prenormalize " CSP_ID_FMT, process);

    /* First find the τ-closure of the initial process. */
    csp_id_set_add(&closure, process);
    csp_process_find_closure(csp, csp->tau, &closure);
    XDEBUG("τ closure is ");
    DEBUG_PROCESS_SET(&closure);

    /* Start processing with the initial process. */
    if (csp_normalized_lts_add_node(lts, &closure, &node)) {
        DEBUG("Register normalized root " CSP_ID_FMT " for " CSP_ID_FMT, node,
              process);
        csp_normalized_lts_add_normalized_root(lts, process, node);
        DEBUG("Enqueue normalized node " CSP_ID_FMT, node);
        csp_id_set_add(current_pending, node);
    }

    /* Keep processing normalized LTS nodes until we run out. */
    while (!csp_id_set_empty(current_pending)) {
        struct csp_id_set_iterator i;
        csp_id_set_clear(next_pending);
        csp_id_set_foreach (current_pending, &i) {
            struct csp_id_set_iterator j;
            csp_id current = csp_id_set_iterator_get(&i);
            const struct csp_id_set *current_processes =
                    csp_normalized_lts_get_node_processes(lts, current);
            DEBUG("Process normalized node " CSP_ID_FMT, current);
            XDEBUG("representing processes ");
            DEBUG_PROCESS_SET(current_processes);

            /* Find all of the non-τ events that any of the current processes
             * can perform. */
            csp_id_set_clear(&initials);
            csp_id_set_foreach (current_processes, &j) {
                csp_id process = csp_id_set_iterator_get(&j);
                DEBUG("Get initials of " CSP_ID_FMT, process);
                csp_build_process_initials(csp, process, &initials);
            }
            csp_id_set_remove(&initials, csp->tau);
            XDEBUG("Merged initials are ");
            DEBUG_EVENT_SET(&initials);

            /* For each of those events, union together the `afters` of that
             * event for all of the processes in the current set. */
            csp_id_set_foreach (&initials, &j) {
                struct csp_id_set_iterator k;
                csp_id initial = csp_id_set_iterator_get(&j);
                csp_id after;
                DEBUG("Get afters for %s", csp_get_event_name(csp, initial));
                csp_id_set_foreach (current_processes, &k) {
                    csp_id process = csp_id_set_iterator_get(&k);
                    DEBUG("Get afters of " CSP_ID_FMT " for %s", process,
                          csp_get_event_name(csp, initial));
                    csp_build_process_afters(csp, process, initial, &closure);
                }

                /* Calculate the τ-closure of that set. */
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
                    csp_id_set_add(next_pending, after);
                }

                /* Add an edge to the normalized LTS. */
                csp_normalized_lts_add_edge(lts, current, initial, after);
            }
        }

        /* Prepare the pending set for the next round. */
        swap(current_pending, next_pending);
    }

    csp_id_set_done(&closure);
    csp_id_set_done(&pending1);
    csp_id_set_done(&pending2);
    csp_id_set_done(&initials);
    return node;
}

bool
csp_check_traces_refinement(struct csp *csp, struct csp_normalized_lts *lts,
                            csp_id normalized_spec, csp_id impl)
{
    struct csp_id_pair_set checked;
    struct csp_id_pair_set checking;
    struct csp_id_pair_set_builder pending;
    struct csp_id_set initials;
    struct csp_id_set afters;
    struct csp_behavior impl_behavior;
    struct csp_id_pair root = {normalized_spec, impl};

    csp_id_pair_set_init(&checked);
    csp_id_pair_set_init(&checking);
    csp_id_pair_set_builder_init(&pending);
    csp_id_set_init(&initials);
    csp_id_set_init(&afters);
    csp_behavior_init(&impl_behavior);
    csp_id_pair_set_builder_add(&pending, root);
    DEBUG("=== check " CSP_ID_FMT " ⊑T " CSP_ID_FMT, normalized_spec, impl);

    while (pending.count > 0) {
        size_t i;
        csp_id_pair_set_build(&checking, &pending);
        DEBUG("--- new round; checking %zu pairs", checking.count);
        for (i = 0; i < checking.count; i++) {
            struct csp_id_set_iterator j;
            const struct csp_id_pair *current = &checking.pairs[i];
            csp_id spec_node = current->from;
            csp_id impl_node = current->to;
            const struct csp_behavior *spec_behavior;

            DEBUG("  check   " CSP_ID_FMT " ⊑T " CSP_ID_FMT, spec_node,
                  impl_node);
            spec_behavior =
                    csp_normalized_lts_get_node_behavior(lts, spec_node);
            XDEBUG("    spec: ");
            DEBUG_EVENT_SET(&spec_behavior->initials);
            csp_process_get_behavior(csp, impl_node, spec_behavior->model,
                                     &impl_behavior);
            XDEBUG("    impl: ");
            DEBUG_EVENT_SET(&impl_behavior.initials);

            if (!csp_behavior_refines(spec_behavior, &impl_behavior)) {
                /* TODO: Construct a counterexample */
                DEBUG("    NOPE");
                goto failure;
            }

            csp_id_set_clear(&initials);
            csp_build_process_initials(csp, impl_node, &initials);
            csp_id_set_foreach (&initials, &j) {
                struct csp_id_set_iterator k;
                csp_id initial = csp_id_set_iterator_get(&j);
                csp_id spec_after;
                DEBUG("    impl -%s→ {...}", csp_get_event_name(csp, initial));
                if (initial == csp->tau) {
                    spec_after = spec_node;
                } else {
                    spec_after = csp_normalized_lts_get_edge(lts, spec_node,
                                                             initial);
                    if (spec_after == CSP_ID_NONE) {
                        /* TODO: Construct a counterexample */
                        DEBUG("      NOPE");
                        goto failure;
                    }
                }
                DEBUG("    spec -%s→ " CSP_ID_FMT,
                      csp_get_event_name(csp, initial), spec_after);

                csp_id_set_clear(&afters);
                csp_build_process_afters(csp, impl_node, initial, &afters);
                csp_id_set_foreach (&afters, &k) {
                    csp_id impl_after = csp_id_set_iterator_get(&k);
                    struct csp_id_pair next = {spec_after, impl_after};
                    DEBUG("    impl -%s→ " CSP_ID_FMT,
                          csp_get_event_name(csp, initial), impl_after);
                    if (!csp_id_pair_set_contains(&checked, next)) {
                        DEBUG("      enqueue (" CSP_ID_FMT "," CSP_ID_FMT ")",
                              spec_after, impl_after);
                        csp_id_pair_set_builder_add(&pending, next);
                    }
                }
            }
        }

        csp_id_pair_set_union(&checked, &checking);
    }

    csp_id_pair_set_done(&checked);
    csp_id_pair_set_done(&checking);
    csp_id_pair_set_builder_done(&pending);
    csp_id_set_done(&initials);
    csp_id_set_done(&afters);
    csp_behavior_done(&impl_behavior);
    return true;

failure:
    csp_id_pair_set_done(&checked);
    csp_id_pair_set_done(&checking);
    csp_id_pair_set_builder_done(&pending);
    csp_id_set_done(&initials);
    csp_id_set_done(&afters);
    csp_behavior_done(&impl_behavior);
    return false;
}

bool
csp_process_check_traces_refinement(struct csp *csp, csp_id spec, csp_id impl)
{
    struct csp_normalized_lts *lts;
    csp_id spec_normalized;
    struct csp_equivalences equiv;
    bool result;

    csp_equivalences_init(&equiv);
    lts = csp_normalized_lts_new(csp, CSP_TRACES);
    spec_normalized = csp_process_prenormalize(csp, lts, spec);
    csp_normalized_lts_bisimulate(lts, &equiv);
    csp_normalized_lts_merge_equivalences(lts, &equiv);
    result = csp_check_traces_refinement(csp, lts, spec_normalized, impl);
    csp_normalized_lts_free(lts);
    csp_equivalences_done(&equiv);
    return result;
}
