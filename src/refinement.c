/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "refinement.h"

#include "ccan/compiler/compiler.h"
#include "behavior.h"
#include "event.h"
#include "id-pair.h"
#include "macros.h"
#include "normalization.h"

#if defined(REFINEMENT_DEBUG)
#include <stdio.h>
#define XDEBUG(...) fprintf(stderr, __VA_ARGS__)
#define XDEBUG_EVENT_SET(set)                            \
    do {                                                 \
        bool __first = true;                             \
        struct csp_event_set_iterator __iter;            \
        XDEBUG("{");                                     \
        csp_event_set_foreach ((set), &__iter) {         \
            const struct csp_event *__event =            \
                    csp_event_set_iterator_get(&__iter); \
            if (__first) {                               \
                __first = false;                         \
            } else {                                     \
                XDEBUG(",");                             \
            }                                            \
            XDEBUG("%s", csp_event_name(__event));       \
        }                                                \
        XDEBUG("}");                                     \
    } while (0)
#define XDEBUG_PROCESS_SET(set)                             \
    do {                                                    \
        bool __first = true;                                \
        struct csp_id_set_iterator __iter;                  \
        XDEBUG("{");                                        \
        csp_id_set_foreach ((set), &__iter) {               \
            csp_id __id = csp_id_set_iterator_get(&__iter); \
            if (__first) {                                  \
                __first = false;                            \
            } else {                                        \
                XDEBUG(",");                                \
            }                                               \
            XDEBUG(CSP_ID_FMT, __id);                       \
        }                                                   \
        XDEBUG("}");                                        \
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

static bool
csp_perform_traces_refinement_check(struct csp *csp,
                                    struct csp_process *normalized_spec,
                                    struct csp_process *impl)
{
    struct csp_id_pair_set checked;
    struct csp_id_pair_set checking;
    struct csp_id_pair_set_builder pending;
    struct csp_event_set initials;
    struct csp_id_set afters;
    struct csp_behavior spec_behavior;
    struct csp_behavior impl_behavior;
    struct csp_id_pair root = {normalized_spec->id, impl->id};

    csp_id_pair_set_init(&checked);
    csp_id_pair_set_init(&checking);
    csp_id_pair_set_builder_init(&pending);
    csp_event_set_init(&initials);
    csp_id_set_init(&afters);
    csp_behavior_init(&spec_behavior);
    csp_behavior_init(&impl_behavior);
    csp_id_pair_set_builder_add(&pending, root);
    DEBUG("=== check " CSP_ID_FMT " ⊑T " CSP_ID_FMT, normalized_spec->id,
          impl->id);

    while (pending.count > 0) {
        size_t i;
        csp_id_pair_set_build(&checking, &pending);
        DEBUG("--- new round; checking %zu pairs", checking.count);
        for (i = 0; i < checking.count; i++) {
            struct csp_event_set_iterator j;
            const struct csp_id_pair *current = &checking.pairs[i];
            csp_id spec_id = current->from;
            csp_id impl_id = current->to;
            struct csp_process *spec = csp_require_process(csp, spec_id);

            DEBUG("  check   " CSP_ID_FMT " ⊑T " CSP_ID_FMT, spec_id, impl_id);
            csp_process_get_behavior(csp, spec_id, CSP_TRACES, &spec_behavior);
            XDEBUG("    spec: ");
            DEBUG_EVENT_SET(&spec_behavior.initials);
            csp_process_get_behavior(csp, impl_id, CSP_TRACES, &impl_behavior);
            XDEBUG("    impl: ");
            DEBUG_EVENT_SET(&impl_behavior.initials);

            if (!csp_behavior_refines(&spec_behavior, &impl_behavior)) {
                /* TODO: Construct a counterexample */
                DEBUG("    NOPE");
                goto failure;
            }

            csp_event_set_clear(&initials);
            csp_build_process_initials(csp, impl_id, &initials);
            csp_event_set_foreach (&initials, &j) {
                struct csp_id_set_iterator k;
                const struct csp_event *initial =
                        csp_event_set_iterator_get(&j);
                const struct csp_process *spec_after;
                DEBUG("    impl -%s→ {...}", csp_event_name(initial));
                if (initial == csp->tau) {
                    spec_after = spec;
                } else {
                    spec_after =
                            csp_process_get_single_after(csp, spec, initial);
                    if (spec_after == NULL) {
                        /* TODO: Construct a counterexample */
                        DEBUG("      NOPE");
                        goto failure;
                    }
                }
                DEBUG("    spec -%s→ " CSP_ID_FMT, csp_event_name(initial),
                      spec_after->id);

                csp_id_set_clear(&afters);
                csp_build_process_afters(csp, impl_id, initial, &afters);
                csp_id_set_foreach (&afters, &k) {
                    csp_id impl_after = csp_id_set_iterator_get(&k);
                    struct csp_id_pair next = {spec_after->id, impl_after};
                    DEBUG("    impl -%s→ " CSP_ID_FMT, csp_event_name(initial),
                          impl_after);
                    if (!csp_id_pair_set_contains(&checked, next)) {
                        DEBUG("      enqueue (" CSP_ID_FMT "," CSP_ID_FMT ")",
                              spec_after->id, impl_after);
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
    csp_event_set_done(&initials);
    csp_id_set_done(&afters);
    csp_behavior_done(&spec_behavior);
    csp_behavior_done(&impl_behavior);
    return true;

failure:
    csp_id_pair_set_done(&checked);
    csp_id_pair_set_done(&checking);
    csp_id_pair_set_builder_done(&pending);
    csp_event_set_done(&initials);
    csp_id_set_done(&afters);
    csp_behavior_done(&spec_behavior);
    csp_behavior_done(&impl_behavior);
    return false;
}

bool
csp_check_traces_refinement(struct csp *csp, struct csp_process *spec,
                            struct csp_process *impl)
{
    struct csp_process *prenormalized;
    struct csp_process *normalized;
    prenormalized = csp_prenormalize_process(csp, spec);
    normalized = csp_normalize_process(csp, prenormalized);
    return csp_perform_traces_refinement_check(csp, normalized, impl);
}
