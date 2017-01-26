/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "refinement.h"

#include "ccan/compiler/compiler.h"
#include "ccan/container_of/container_of.h"
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
#define XDEBUG_PROCESS(process)                                 \
    do {                                                        \
        struct csp_print_name __print = csp_print_name(stderr); \
        csp_process_name(csp, (process), &__print.visitor);     \
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
#define XDEBUG_PROCESS(process) /* do nothing */
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
#define DEBUG_PROCESS(process)   \
    do {                         \
        XDEBUG_PROCESS(process); \
        XDEBUG("\n");            \
    } while (0)
#define DEBUG_PROCESS_SET(set)   \
    do {                         \
        XDEBUG_PROCESS_SET(set); \
        XDEBUG("\n");            \
    } while (0)

/*------------------------------------------------------------------------------
 * Refinement check process
 */

struct csp_refinement_process {
    struct csp_process process;
    struct csp_process *spec;
    struct csp_process *impl;
};

static void
csp_refinement_process_name(struct csp *csp, struct csp_process *process,
                            struct csp_name_visitor *visitor)
{
    struct csp_refinement_process *refinement =
            container_of(process, struct csp_refinement_process, process);
    csp_process_nested_name(csp, process, refinement->spec, visitor);
    csp_name_visitor_call(csp, visitor, " ⊑ ");
    csp_process_nested_name(csp, process, refinement->impl, visitor);
}

static void
csp_refinement_process_initials(struct csp *csp, struct csp_process *process,
                                struct csp_event_visitor *visitor)
{
    struct csp_refinement_process *refinement =
            container_of(process, struct csp_refinement_process, process);
    csp_process_visit_initials(csp, refinement->impl, visitor);
}

struct csp_add_spec {
    struct csp_edge_visitor visitor;
    struct csp_process *spec_after;
    struct csp_edge_visitor *wrapped;
};

static void
csp_add_spec_visit(struct csp *csp, struct csp_edge_visitor *visitor,
                   const struct csp_event *initial,
                   struct csp_process *impl_after)
{
    struct csp_add_spec *self =
            container_of(visitor, struct csp_add_spec, visitor);
    struct csp_process *refinement_after =
        csp_refinement_process(csp, self->spec_after, impl_after);
    XDEBUG("    impl -%s→ ", csp_event_name(initial));
    DEBUG_PROCESS(impl_after);
    csp_edge_visitor_call(csp, self->wrapped, initial, refinement_after);
}

struct csp_add_spec
csp_add_spec(struct csp_process *spec_after, struct csp_edge_visitor *wrapped)
{
    struct csp_add_spec self = {{csp_add_spec_visit}, spec_after, wrapped};
    return self;
}

static void
csp_refinement_process_afters(struct csp *csp, struct csp_process *process,
                              const struct csp_event *initial,
                              struct csp_edge_visitor *visitor)
{
    struct csp_refinement_process *refinement =
            container_of(process, struct csp_refinement_process, process);
    struct csp_process *spec_after;
    struct csp_add_spec add_spec;
    DEBUG("    impl -%s→ {...}", csp_event_name(initial));
    if (initial == csp->tau) {
        spec_after = refinement->spec;
    } else {
        spec_after =
                csp_process_get_single_after(csp, refinement->spec, initial);
        if (spec_after == NULL) {
            DEBUG("      NOPE");
            return;
        }
    }
    XDEBUG("    spec -%s→ ", csp_event_name(initial));
    DEBUG_PROCESS(spec_after);
    add_spec = csp_add_spec(spec_after, visitor);
    csp_process_visit_afters(csp, refinement->impl, initial, &add_spec.visitor);
}

static void
csp_refinement_process_free(struct csp *csp, struct csp_process *process)
{
    struct csp_refinement_process *refinement =
            container_of(process, struct csp_refinement_process, process);
    free(refinement);
}

static const struct csp_process_iface csp_refinement_process_iface = {
        0, csp_refinement_process_name, csp_refinement_process_initials,
        csp_refinement_process_afters, csp_refinement_process_free};

static csp_id
csp_refinement_process_get_id(struct csp_process *spec,
                              struct csp_process *impl)
{
    static struct csp_id_scope refinement_process;
    csp_id id = csp_id_start(&refinement_process);
    id = csp_id_add_process(id, spec);
    id = csp_id_add_process(id, impl);
    return id;
}

struct csp_process *
csp_refinement_process(struct csp *csp, struct csp_process *spec,
                       struct csp_process *impl)
{
    csp_id id = csp_refinement_process_get_id(spec, impl);
    struct csp_refinement_process *refinement;
    return_if_nonnull(csp_get_process(csp, id));
    refinement = malloc(sizeof(struct csp_refinement_process));
    assert(refinement != NULL);
    refinement->process.id = id;
    refinement->process.iface = &csp_refinement_process_iface;
    refinement->spec = spec;
    refinement->impl = impl;
    csp_register_process(csp, &refinement->process);
    return &refinement->process;
}

static struct csp_refinement_process *
csp_refinement_process_downcast(struct csp_process *process)
{
    assert(process->iface == &csp_refinement_process_iface);
    return container_of(process, struct csp_refinement_process, process);
}

/*------------------------------------------------------------------------------
 * Refinement
 */

struct csp_enqueue_next {
    struct csp_edge_visitor visitor;
    struct csp_process_set *enqueued;
    struct csp_process_set *pending;
    bool any_next;
};

static void
csp_enqueue_next_visit(struct csp *csp, struct csp_edge_visitor *visitor,
                       const struct csp_event *initial,
                       struct csp_process *after)
{
    struct csp_enqueue_next *self =
            container_of(visitor, struct csp_enqueue_next, visitor);
    self->any_next = true;
    if (csp_process_set_add(self->enqueued, after)) {
        UNNEEDED struct csp_refinement_process *refinement_after =
                csp_refinement_process_downcast(after);
        XDEBUG("      enqueue (");
        XDEBUG_PROCESS(refinement_after->spec);
        XDEBUG(",");
        XDEBUG_PROCESS(refinement_after->impl);
        DEBUG(")");
        csp_process_set_add(self->pending, after);
    }
}

static struct csp_enqueue_next
csp_enqueue_next(struct csp_process_set *enqueued,
                 struct csp_process_set *pending)
{
    struct csp_enqueue_next self = {
            {csp_enqueue_next_visit}, enqueued, pending, false};
    return self;
}

struct csp_check_refinement_initials {
    struct csp_event_visitor visitor;
    struct csp_process *process;
    struct csp_process_set *enqueued;
    struct csp_process_set *pending;
    bool refinement_holds;
};

static void
csp_check_refinement_initials_visit(struct csp *csp,
                                    struct csp_event_visitor *visitor,
                                    const struct csp_event *initial)
{
    struct csp_check_refinement_initials *self = container_of(
            visitor, struct csp_check_refinement_initials, visitor);
    struct csp_enqueue_next enqueue =
            csp_enqueue_next(self->enqueued, self->pending);
    csp_process_visit_afters(csp, self->process, initial, &enqueue.visitor);
    if (!enqueue.any_next) {
        DEBUG("      NOPE");
        self->refinement_holds = false;
    }
}

static struct csp_check_refinement_initials
csp_check_refinement_initials(struct csp_process *process,
                              struct csp_process_set *enqueued,
                              struct csp_process_set *pending)
{
    struct csp_check_refinement_initials self = {
            {csp_check_refinement_initials_visit},
            process,
            enqueued,
            pending,
            true};
    return self;
}

static bool
csp_check_refinement_process(struct csp *csp,
                             struct csp_refinement_process *refinement,
                             struct csp_process_set *enqueued,
                             struct csp_process_set *pending)
{
    struct csp_behavior spec_behavior;
    struct csp_behavior impl_behavior;
    struct csp_check_refinement_initials check_initials;

    csp_behavior_init(&spec_behavior);
    csp_behavior_init(&impl_behavior);
    csp_process_get_behavior(csp, refinement->spec, CSP_TRACES, &spec_behavior);
    csp_process_get_behavior(csp, refinement->impl, CSP_TRACES, &impl_behavior);
    XDEBUG("  check ");
    XDEBUG_PROCESS(refinement->spec);
    XDEBUG(" ⊑ ");
    DEBUG_PROCESS(refinement->impl);
    XDEBUG("    spec: ");
    DEBUG_EVENT_SET(&spec_behavior.initials);
    XDEBUG("    impl: ");
    DEBUG_EVENT_SET(&impl_behavior.initials);
    if (!csp_behavior_refines(&spec_behavior, &impl_behavior)) {
        /* TODO: Construct a counterexample */
        DEBUG("    NOPE");
        csp_behavior_done(&spec_behavior);
        csp_behavior_done(&impl_behavior);
        return false;
    }
    csp_behavior_done(&spec_behavior);
    csp_behavior_done(&impl_behavior);

    check_initials = csp_check_refinement_initials(&refinement->process,
                                                   enqueued, pending);
    csp_process_visit_initials(csp, &refinement->process,
                               &check_initials.visitor);
    return check_initials.refinement_holds;
}

static bool
csp_check_refinement_process_set(struct csp *csp,
                                 struct csp_process_set *enqueued,
                                 struct csp_process_set *checking,
                                 struct csp_process_set *pending)
{
    struct csp_process_set_iterator iter;
    csp_process_set_foreach (checking, &iter) {
        struct csp_process *process = csp_process_set_iterator_get(&iter);
        struct csp_refinement_process *refinement =
                csp_refinement_process_downcast(process);
        if (!csp_check_refinement_process(csp, refinement, enqueued, pending)) {
            return false;
        }
    }
    return true;
}

static bool
csp_perform_traces_refinement_check(struct csp *csp,
                                    struct csp_process *refinement)
{
    struct csp_process_set enqueued;
    struct csp_process_set set1;
    struct csp_process_set set2;
    struct csp_process_set *checking;
    struct csp_process_set *pending;

    csp_process_set_init(&enqueued);
    csp_process_set_init(&set1);
    csp_process_set_init(&set2);
    checking = &set1;
    pending = &set2;
    csp_process_set_add(pending, refinement);
    XDEBUG("=== check ");
    DEBUG_PROCESS(refinement);

    while (!csp_process_set_empty(pending)) {
        swap(checking, pending);
        csp_process_set_clear(pending);
        DEBUG("--- new round; checking %zu pairs",
              csp_process_set_size(checking));
        if (!csp_check_refinement_process_set(csp, &enqueued, checking,
                                              pending)) {
            goto failure;
        }
    }

    csp_process_set_done(&enqueued);
    csp_process_set_done(&set1);
    csp_process_set_done(&set2);
    return true;

failure:
    csp_process_set_done(&enqueued);
    csp_process_set_done(&set1);
    csp_process_set_done(&set2);
    return false;
}

bool
csp_check_traces_refinement(struct csp *csp, struct csp_process *spec,
                            struct csp_process *impl)
{
    struct csp_process *prenormalized;
    struct csp_process *normalized;
    struct csp_process *refinement;
    prenormalized = csp_prenormalize_process(csp, spec);
    normalized = csp_normalize_process(csp, prenormalized);
    refinement = csp_refinement_process(csp, normalized, impl);
    return csp_perform_traces_refinement_check(csp, refinement);
}
