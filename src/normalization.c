/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "normalization.h"

#include <assert.h>

#include "ccan/container_of/container_of.h"
#include "basics.h"
#include "behavior.h"
#include "environment.h"
#include "equivalence.h"
#include "event.h"
#include "id-set.h"
#include "macros.h"
#include "process.h"

#if defined(NORMALIZATION_DEBUG)
#include <stdio.h>
#define XDEBUG(...) fprintf(stderr, __VA_ARGS__)
#define XDEBUG_PROCESS_SET(set)                            \
    do {                                                   \
        bool __first = true;                               \
        struct csp_process_set_iterator __iter;            \
        XDEBUG("{");                                       \
        csp_process_set_foreach((set), &__iter)            \
        {                                                  \
            struct csp_process *__process =                \
                    csp_process_set_iterator_get(&__iter); \
            if (__first) {                                 \
                __first = false;                           \
            } else {                                       \
                XDEBUG(",");                               \
            }                                              \
            XDEBUG(CSP_ID_FMT, __process->id);             \
        }                                                  \
        XDEBUG("}");                                       \
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

/*------------------------------------------------------------------------------
 * Process closures
 */

void
csp_find_process_closure(struct csp *csp, const struct csp_event *event,
                         struct csp_process_set *processes)
{
    bool another_round_needed = true;
    struct csp_process_set queue1;
    struct csp_process_set queue2;
    struct csp_process_set *current_queue = &queue1;
    struct csp_process_set *next_queue = &queue2;
    csp_process_set_init(&queue1);
    csp_process_set_init(&queue2);
    csp_process_set_union(current_queue, processes);
    XDEBUG("=== closure of ");
    DEBUG_PROCESS_SET(processes);
    while (another_round_needed) {
        struct csp_process_set_iterator i;
        DEBUG("--- start closure iteration %p", current_queue);
        csp_process_set_clear(next_queue);
        csp_process_set_foreach (current_queue, &i) {
            struct csp_process *process = csp_process_set_iterator_get(&i);
            struct csp_collect_afters collect = csp_collect_afters(next_queue);
            DEBUG("process " CSP_ID_FMT, process->id);
            /* Enqueue each of the states that we can reach from `process` by
             * following a single `event`. */
            csp_process_visit_afters(csp, process, event, &collect.visitor);
        }
        another_round_needed = csp_process_set_union(processes, next_queue);
        swap(current_queue, next_queue);
    }
    csp_process_set_done(&queue1);
    csp_process_set_done(&queue2);
}

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process_get_single_after {
    struct csp_edge_visitor visitor;
    struct csp_process *after;
};

static void
csp_process_get_single_after_visit_edge(struct csp *csp,
                                        struct csp_edge_visitor *visitor,
                                        const struct csp_event *initial,
                                        struct csp_process *after)
{
    struct csp_process_get_single_after *self =
            container_of(visitor, struct csp_process_get_single_after, visitor);
    assert(self->after == NULL);
    self->after = after;
}

struct csp_process *
csp_process_get_single_after(struct csp *csp, struct csp_process *process,
                             const struct csp_event *initial)
{
    struct csp_process_get_single_after self = {
            {csp_process_get_single_after_visit_edge}, NULL};
    csp_process_visit_afters(csp, process, initial, &self.visitor);
    return self.after;
}

/*------------------------------------------------------------------------------
 * Prenormalized process
 */

struct csp_prenormalized_process {
    struct csp_process process;
    struct csp_process_set ps; /* Must be τ-closed */
};

/* `ps` must be τ-closed */
struct csp_process *
csp_prenormalized_process_new(struct csp *csp,
                              const struct csp_process_set *ps);

static void
csp_prenormalized_process_name(struct csp *csp, struct csp_process *process,
                               struct csp_name_visitor *visitor)
{
    struct csp_prenormalized_process *self =
            container_of(process, struct csp_prenormalized_process, process);
    csp_name_visitor_call(csp, visitor, "prenormalized ");
    csp_process_set_name(csp, &self->ps, visitor);
}

static void
csp_prenormalized_process_initials(struct csp *csp, struct csp_process *process,
                                   struct csp_event_visitor *visitor)
{
    /* Find all of the non-τ events that any of the underlying processes can
     * perform. */
    struct csp_prenormalized_process *self =
            container_of(process, struct csp_prenormalized_process, process);
    struct csp_ignore_event ignore = csp_ignore_event(visitor, csp->tau);
    struct csp_process_set_iterator iter;
    csp_process_set_foreach (&self->ps, &iter) {
        struct csp_process *subprocess = csp_process_set_iterator_get(&iter);
        csp_process_visit_initials(csp, subprocess, &ignore.visitor);
    }
}

static void
csp_prenormalized_process_afters(struct csp *csp, struct csp_process *process,
                                 const struct csp_event *initial,
                                 struct csp_edge_visitor *visitor)
{
    struct csp_prenormalized_process *self =
            container_of(process, struct csp_prenormalized_process, process);
    struct csp_process_set afters;
    struct csp_process_set_iterator iter;
    struct csp_process *after;

    /* Normalized processes can never perform a τ. */
    if (initial == csp->tau) {
        return;
    }

    /* Find the set of processes that you could end up in by starting in one of
     * our underlying processes and following a single `initial` event. */
    csp_process_set_init(&afters);
    csp_process_set_foreach (&self->ps, &iter) {
        struct csp_process *subprocess = csp_process_set_iterator_get(&iter);
        struct csp_collect_afters collect = csp_collect_afters(&afters);
        csp_process_visit_afters(csp, subprocess, initial, &collect.visitor);
    }

    /* Since a normalized process can only have one `after` for any event, merge
     * together all of the possible afters into a single normalized process. */
    csp_find_process_closure(csp, csp->tau, &afters);
    after = csp_prenormalized_process_new(csp, &afters);
    csp_process_set_done(&afters);
    csp_edge_visitor_call(csp, visitor, initial, after);
}

static void
csp_prenormalized_process_free(struct csp *csp, struct csp_process *process)
{
    struct csp_prenormalized_process *self =
            container_of(process, struct csp_prenormalized_process, process);
    csp_process_set_done(&self->ps);
    free(self);
}

static const struct csp_process_iface csp_prenormalized_process_iface = {
        0, csp_prenormalized_process_name, csp_prenormalized_process_initials,
        csp_prenormalized_process_afters, csp_prenormalized_process_free};

static csp_id
csp_prenormalized_process_get_id(const struct csp_process_set *ps)
{
    static struct csp_id_scope prenormalized_process;
    csp_id id = csp_id_start(&prenormalized_process);
    id = csp_id_add_process_set(id, ps);
    return id;
}

struct csp_process *
csp_prenormalized_process_new(struct csp *csp, const struct csp_process_set *ps)
{
    struct csp_prenormalized_process *self;
    csp_id id = csp_prenormalized_process_get_id(ps);
    return_if_nonnull(csp_get_process(csp, id));
    self = malloc(sizeof(struct csp_prenormalized_process));
    assert(self != NULL);
    self->process.id = id;
    self->process.iface = &csp_prenormalized_process_iface;
    csp_process_set_init(&self->ps);
    csp_process_set_union(&self->ps, ps);
    csp_register_process(csp, &self->process);
    return &self->process;
}

struct csp_process *
csp_prenormalize_process(struct csp *csp, struct csp_process *subprocess)
{
    struct csp_process_set ps;
    struct csp_process *process;
    csp_process_set_init(&ps);
    csp_process_set_add(&ps, subprocess);
    csp_find_process_closure(csp, csp->tau, &ps);
    process = csp_prenormalized_process_new(csp, &ps);
    csp_process_set_done(&ps);
    return process;
}

static struct csp_prenormalized_process *
csp_prenormalized_process_downcast(struct csp_process *process)
{
    assert(process->iface == &csp_prenormalized_process_iface);
    return container_of(process, struct csp_prenormalized_process, process);
}

const struct csp_process_set *
csp_prenormalized_process_get_processes(struct csp_process *process)
{
    struct csp_prenormalized_process *self =
            csp_prenormalized_process_downcast(process);
    return &self->ps;
}

/*------------------------------------------------------------------------------
 * Step equivalence
 */

/* Check whether two processes that we previously assumed were equivalent are
 * still equivalent.  Two processes (which we assume are currently equivalent)
 * should continue to be equivalent if all of the targets from both lead to
 * processes that are themselves equivalent. */

struct csp_processes_equiv {
    struct csp_edge_visitor visitor;
    struct csp_equivalences *equiv;
    struct csp_process *p1;
    struct csp_process *p2;
    bool equivalent;
};

static bool
csp_equivalences_equiv(struct csp_equivalences *equiv, struct csp_process *p1,
                       struct csp_process *p2)
{
    csp_id  class1 = csp_equivalences_get_class(equiv, p1->id);
    csp_id  class2 = csp_equivalences_get_class(equiv, p2->id);
    assert(class1 != CSP_ID_NONE);
    assert(class2 != CSP_ID_NONE);
    DEBUG("      " CSP_ID_FMT " ∈ " CSP_ID_FMT, p1->id, class1);
    DEBUG("      " CSP_ID_FMT " ∈ " CSP_ID_FMT, p2->id, class2);
    return class1 == class2;
}

static void
csp_processes_equiv_visit_edge(struct csp *csp,
                               struct csp_edge_visitor *visitor,
                               const struct csp_event *event,
                               struct csp_process *after1)
{
    struct csp_processes_equiv *self = container_of(
            visitor, struct csp_processes_equiv, visitor);
    struct csp_process *after2 =
            csp_process_get_single_after(csp, self->p2, event);
    assert(after2 != NULL);
    DEBUG("    --- %s", csp_event_name(event));
    DEBUG("    " CSP_ID_FMT " -%s→ " CSP_ID_FMT, self->p1->id,
          csp_event_name(event), after1->id);
    DEBUG("    " CSP_ID_FMT " -%s→ " CSP_ID_FMT, self->p2->id,
          csp_event_name(event), after2->id);
    /* If after1 and after2 are not equivalent, then to1 and to2 should no
     * longer be equivalent. */
    if (!csp_equivalences_equiv(self->equiv, after1, after2)) {
        DEBUG("  NOT EQUIVALENT");
        self->equivalent = false;
    }
}

static bool
csp_processes_equiv(struct csp *csp, struct csp_equivalences *equiv,
                    struct csp_process *p1, struct csp_process *p2)
{
    struct csp_processes_equiv self = {
            {csp_processes_equiv_visit_edge}, equiv, p1, p2, true};
    assert(p1 != p2);
    DEBUG("  check " CSP_ID_FMT " ?~ " CSP_ID_FMT, p1->id, p2->id);
    csp_process_visit_transitions(csp, p1, &self.visitor);
    if (self.equivalent) {
        DEBUG("  EQUIVALENT");
    }
    return self.equivalent;
}

/*------------------------------------------------------------------------------
 * Bisimulation
 */

struct csp_init_bisimulation {
    struct csp_process_visitor visitor;
    struct csp_behavior behavior;
    struct csp_equivalences *equiv;
};

static void
csp_init_bisimulation_visit_process(struct csp *csp,
                                    struct csp_process_visitor *visitor,
                                    struct csp_process *process)
{
    /* We start by assuming that all nodes with the same behavior are
     * equivalent. */
    struct csp_init_bisimulation *self = container_of(
            visitor, struct csp_init_bisimulation, visitor);
    csp_process_get_behavior(csp, process, CSP_TRACES, &self->behavior);
    DEBUG("  init " CSP_ID_FMT " ⇒ " CSP_ID_FMT, process->id,
          self->behavior.hash);
    csp_equivalences_add(self->equiv, self->behavior.hash, process->id);
}

static void
csp_init_bisimulation(struct csp *csp, struct csp_process *process,
                      struct csp_equivalences *equiv)
{
    struct csp_init_bisimulation self;
    DEBUG("=== initialize");
    self.visitor.visit = csp_init_bisimulation_visit_process;
    csp_behavior_init(&self.behavior);
    self.equiv = equiv;
    csp_process_bfs(csp, process, &self.visitor);
    csp_behavior_done(&self.behavior);
}

void
csp_calculate_bisimulation(struct csp *csp, struct csp_process *prenormalized,
                           struct csp_equivalences *equiv)
{
    struct csp_id_set classes;
    struct csp_id_set members;
    struct csp_equivalences new_equiv;
    struct csp_equivalences *prev_equiv = equiv;
    struct csp_equivalences *next_equiv = &new_equiv;
    bool changed;

    csp_id_set_init(&classes);
    csp_id_set_init(&members);
    csp_equivalences_init(&new_equiv);
    csp_init_bisimulation(csp, prenormalized, prev_equiv);

    do {
        struct csp_id_set_iterator i;
        DEBUG("=== starting new iteration");

        /* We don't want to start another iteration after this one unless we
         * find any changes. */
        changed = false;

        /* Loop through each pair of states that were equivalent before,
         * verifying that they're still equivalent.  Separate any that are not
         * equivalent to their head into a new class. */
        csp_id_set_clear(&classes);
        csp_equivalences_build_classes(prev_equiv, &classes);
        csp_id_set_foreach (&classes, &i) {
            struct csp_id_set_iterator j;
            size_t j_index;
            csp_id class_id = csp_id_set_iterator_get(&i);
            csp_id head_id;
            struct csp_process *head;
            csp_id new_class_id = CSP_ID_NONE;
            DEBUG("class " CSP_ID_FMT, class_id);

            csp_id_set_clear(&members);
            csp_equivalences_build_members(prev_equiv, class_id, &members);

            /* The "head" of this class is just the one that happens to be first
             * in the list of members. */
            csp_id_set_get_iterator(&members, &j);
            assert(!csp_id_set_iterator_done(&j));
            head_id = csp_id_set_iterator_get(&j);
            head = csp_require_process(csp, head_id);
            DEBUG("member[0] = " CSP_ID_FMT, head_id);
            csp_equivalences_add(next_equiv, class_id, head_id);

            /* If we find a non-equivalent member of this class, we'll need to
             * separate it out into a new class.  This new class will need a
             * head, which will be the first non-equivalent member we find.
             *
             * If we find multiple members that aren't equivalent to the head,
             * we'll put them into the same new equivalence class; if they turn
             * out to also not be equivalent to each other, we'll catch that in
             * a later iteration. */
            for (csp_id_set_iterator_advance(&j), j_index = 1;
                 !csp_id_set_iterator_done(&j);
                 csp_id_set_iterator_advance(&j), j_index++) {
                csp_id member_id = csp_id_set_iterator_get(&j);
                struct csp_process *member =
                        csp_require_process(csp, member_id);
                DEBUG("member[%zu] = " CSP_ID_FMT, j_index, member_id);
                if (!csp_processes_equiv(csp, prev_equiv, head, member)) {
                    /* This state is not equivalent to its head.  If necessary,
                     * create a new equivalence class.  Add the node to this new
                     * class. */
                    if (new_class_id == CSP_ID_NONE) {
                        new_class_id = member_id;
                    }
                    DEBUG("  move " CSP_ID_FMT " ⇒ " CSP_ID_FMT, member_id,
                          new_class_id);
                    csp_equivalences_add(next_equiv, new_class_id,
                                         member_id);
                    changed = true;
                } else {
                    DEBUG("  keep " CSP_ID_FMT " ⇒ " CSP_ID_FMT, member_id,
                          class_id);
                    csp_equivalences_add(next_equiv, class_id, member_id);
                }
            }
        }

        swap(prev_equiv, next_equiv);
    } while (changed);

    csp_id_set_done(&classes);
    csp_id_set_done(&members);
    csp_equivalences_done(&new_equiv);
}

/*------------------------------------------------------------------------------
 * Normalized process
 */

struct csp_normalized_process {
    struct csp_process process;
    struct csp_process *prenormalized_root;
    struct csp_id_set subprocess_ids; /* Should be prenormalized processes */
    struct csp_equivalences *equiv;
    csp_id equivalence_class;
    bool equiv_owned;
};

static struct csp_process *
csp_normalized_process_new(struct csp *csp,
                           struct csp_process *prenormalized_root,
                           struct csp_equivalences *equiv,
                           csp_id equivalence_class, bool equiv_owned);

static void
csp_normalized_process_name(struct csp *csp, struct csp_process *process,
                            struct csp_name_visitor *visitor)
{
    struct csp_process_set merged;
    csp_process_set_init(&merged);
    csp_normalized_process_get_processes(csp, process, &merged);
    csp_process_set_name(csp, &merged, visitor);
    csp_process_set_done(&merged);
}

static void
csp_normalized_process_initials(struct csp *csp, struct csp_process *process,
                                struct csp_event_visitor *visitor)
{
    struct csp_normalized_process *self =
            container_of(process, struct csp_normalized_process, process);
    struct csp_ignore_event ignore = csp_ignore_event(visitor, csp->tau);
    struct csp_id_set_iterator iter;
    csp_id_set_foreach (&self->subprocess_ids, &iter) {
        csp_id subprocess_id = csp_id_set_iterator_get(&iter);
        struct csp_process *subprocess =
                csp_require_process(csp, subprocess_id);
        csp_process_visit_initials(csp, subprocess, &ignore.visitor);
    }
}

static void
csp_normalized_process_afters(struct csp *csp, struct csp_process *process,
                              const struct csp_event *initial,
                              struct csp_edge_visitor *visitor)
{
    struct csp_normalized_process *self =
            container_of(process, struct csp_normalized_process, process);
    struct csp_id_set_iterator i;
    struct csp_process_set_iterator p;
    struct csp_process_set afters;
    csp_id equivalence_class;
    struct csp_process *after;

    /* Find the set of processes that you could end up in by starting in one of
     * our underlying processes and following a single `initial` event. */
    csp_process_set_init(&afters);
    csp_id_set_foreach (&self->subprocess_ids, &i) {
        csp_id subprocess_id = csp_id_set_iterator_get(&i);
        struct csp_process *subprocess =
                csp_require_process(csp, subprocess_id);
        struct csp_collect_afters collect = csp_collect_afters(&afters);
        csp_process_visit_afters(csp, subprocess, initial, &collect.visitor);
    }

    /* Because we've already prenormalized the underlying processes and merged
     * equivalent processes via bisimulation, all of the `afters` that we just
     * found should all belong to the same equivalence class. */
    equivalence_class = CSP_ID_NONE;
    csp_process_set_foreach (&afters, &p) {
        struct csp_process *after = csp_process_set_iterator_get(&p);
        csp_id after_equivalence_class =
                csp_equivalences_get_class(self->equiv, after->id);
        assert(equivalence_class == CSP_ID_NONE ||
               equivalence_class == after_equivalence_class);
        equivalence_class = after_equivalence_class;
    }
    csp_process_set_done(&afters);

    /* Our "real" after is the normalized node for this equivalence class that
     * we just found. */
    after = csp_normalized_process_new(csp, self->prenormalized_root,
                                       self->equiv, equivalence_class, false);
    return csp_edge_visitor_call(csp, visitor, initial, after);
}

static void
csp_normalized_process_free(struct csp *csp, struct csp_process *process)
{
    struct csp_normalized_process *self =
            container_of(process, struct csp_normalized_process, process);
    csp_id_set_done(&self->subprocess_ids);
    if (self->equiv_owned) {
        csp_equivalences_free(self->equiv);
    }
    free(self);
}

static const struct csp_process_iface csp_normalized_process_iface = {
        0, csp_normalized_process_name, csp_normalized_process_initials,
        csp_normalized_process_afters, csp_normalized_process_free};

static csp_id
csp_normalized_process_get_id(struct csp_process *prenormalized_root,
                              csp_id equivalence_class)
{
    static struct csp_id_scope normalized_process;
    csp_id id = csp_id_start(&normalized_process);
    id = csp_id_add_process(id, prenormalized_root);
    id = csp_id_add_id(id, equivalence_class);
    return id;
}

static struct csp_process *
csp_normalized_process_new(struct csp *csp,
                           struct csp_process *prenormalized_root,
                           struct csp_equivalences *equiv,
                           csp_id equivalence_class, bool equiv_owned)
{
    struct csp_normalized_process *self;
    struct csp_process *process;
    csp_id id = csp_normalized_process_get_id(prenormalized_root,
                                              equivalence_class);
    process = csp_get_process(csp, id);
    if (unlikely(process != NULL)) {
        if (equiv_owned) {
            csp_equivalences_free(equiv);
        }
        return process;
    }

    self = malloc(sizeof(struct csp_normalized_process));
    assert(self != NULL);
    self->process.id = id;
    self->process.iface = &csp_normalized_process_iface;
    self->prenormalized_root = prenormalized_root;
    self->equiv = equiv;
    self->equiv_owned = equiv_owned;
    self->equivalence_class = equivalence_class;
    csp_id_set_init(&self->subprocess_ids);
    csp_equivalences_build_members(equiv, equivalence_class,
                                   &self->subprocess_ids);
    csp_register_process(csp, &self->process);
    return &self->process;
}

struct csp_process *
csp_normalize_process(struct csp *csp, struct csp_process *prenormalized)
{
    struct csp_equivalences *equiv = csp_equivalences_new();
    csp_id equivalence_class;
    csp_calculate_bisimulation(csp, prenormalized, equiv);
    equivalence_class = csp_equivalences_get_class(equiv, prenormalized->id);
    assert(equivalence_class != CSP_ID_NONE);
    return csp_normalized_process_new(csp, prenormalized, equiv,
                                      equivalence_class, true);
}

static struct csp_normalized_process *
csp_normalized_process_downcast(struct csp_process *process)
{
    assert(process->iface == &csp_normalized_process_iface);
    return container_of(process, struct csp_normalized_process, process);
}

struct csp_process *
csp_normalized_subprocess(struct csp *csp, struct csp_process *root_,
                          struct csp_process *prenormalized)
{
    struct csp_normalized_process *root =
            csp_normalized_process_downcast(root_);
    csp_id class_id;
    /* Figure out which equivalence class `prenormalized` belongs to. */
    class_id = csp_equivalences_get_class(root->equiv, prenormalized->id);
    /* Then return the normalized subprocess for that equivalence class. */
    return csp_normalized_process_new(csp, root->prenormalized_root,
                                      root->equiv, class_id, false);
}

void
csp_normalized_process_get_processes(struct csp *csp,
                                     struct csp_process *process,
                                     struct csp_process_set *set)
{
    struct csp_id_set_iterator iter;
    struct csp_normalized_process *self =
            csp_normalized_process_downcast(process);
    /* self->ps is the set of prenormalized processes that this normalized
     * process represents.  We need to grab the processes that each of those
     * represent to get our final answer. */
    csp_id_set_foreach (&self->subprocess_ids, &iter) {
        csp_id subprocess_id = csp_id_set_iterator_get(&iter);
        struct csp_process *subprocess =
                csp_require_process(csp, subprocess_id);
        csp_process_set_union(
                set, csp_prenormalized_process_get_processes(subprocess));
    }
}
