/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "process.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/container_of/container_of.h"
#include "ccan/likely/likely.h"
#include "basics.h"
#include "environment.h"
#include "event.h"
#include "macros.h"

/*------------------------------------------------------------------------------
 * Edge visitors
 */

void
csp_edge_visitor_call(struct csp *csp, struct csp_edge_visitor *visitor,
                      const struct csp_event *event, struct csp_process *after)
{
    visitor->visit(csp, visitor, event, after);
}

static void
csp_collect_afters_visit(struct csp *csp, struct csp_edge_visitor *visitor,
                         const struct csp_event *event,
                         struct csp_process *after)
{
    struct csp_collect_afters *self =
            container_of(visitor, struct csp_collect_afters, visitor);
    csp_process_set_add(self->set, after);
}

struct csp_collect_afters
csp_collect_afters(struct csp_process_set *set)
{
    struct csp_collect_afters self = {{csp_collect_afters_visit}, set};
    return self;
}

/*------------------------------------------------------------------------------
 * Process visitors
 */

void
csp_process_visitor_call(struct csp *csp, struct csp_process_visitor *visitor,
                         struct csp_process *process)
{
    visitor->visit(csp, visitor, process);
}

static void
csp_collect_processes_visit(struct csp *csp,
                            struct csp_process_visitor *visitor,
                            struct csp_process *process)
{
    struct csp_collect_processes *self =
            container_of(visitor, struct csp_collect_processes, visitor);
    csp_process_set_add(self->set, process);
}

struct csp_collect_processes
csp_collect_processes(struct csp_process_set *set)
{
    struct csp_collect_processes self = {{csp_collect_processes_visit}, set};
    return self;
}

/*------------------------------------------------------------------------------
 * Name visitor
 */

void
csp_name_visitor_call(struct csp *csp, struct csp_name_visitor *visitor,
                      const char *str)
{
    visitor->visit(csp, visitor, str, strlen(str));
}

void
csp_name_visitor_call_sized(struct csp *csp, struct csp_name_visitor *visitor,
                            const char *str, size_t length)
{
    visitor->visit(csp, visitor, str, length);
}

static void
csp_print_name_visit(struct csp *csp, struct csp_name_visitor *visitor,
                     const char *str, size_t length)
{
    struct csp_print_name *self =
            container_of(visitor, struct csp_print_name, visitor);
    fwrite(str, length, 1, self->out);
}

struct csp_print_name
csp_print_name(FILE *out)
{
    struct csp_print_name self = {{csp_print_name_visit}, out};
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
csp_process_name(struct csp *csp, struct csp_process *process,
                 struct csp_name_visitor *visitor)
{
    process->iface->name(csp, process, visitor);
}

void
csp_process_nested_name(struct csp *csp, struct csp_process *process,
                        struct csp_process *subprocess,
                        struct csp_name_visitor *visitor)
{
    if (process->iface->precedence < subprocess->iface->precedence) {
        csp_name_visitor_call(csp, visitor, "(");
        subprocess->iface->name(csp, subprocess, visitor);
        csp_name_visitor_call(csp, visitor, ")");
    } else {
        subprocess->iface->name(csp, subprocess, visitor);
    }
}

void
csp_process_visit_initials(struct csp *csp, struct csp_process *process,
                           struct csp_event_visitor *visitor)
{
    process->iface->initials(csp, process, visitor);
}

void
csp_process_visit_afters(struct csp *csp, struct csp_process *process,
                         const struct csp_event *initial,
                         struct csp_edge_visitor *visitor)
{
    process->iface->afters(csp, process, initial, visitor);
}

struct csp_process_visit_transitions {
    struct csp_process *process;
    struct csp_edge_visitor *wrapped;
    struct csp_event_visitor visit_initial;
};

static void
csp_process_visit_transitions_visit_initial(struct csp *csp,
                                            struct csp_event_visitor *visitor,
                                            const struct csp_event *initial)
{
    struct csp_process_visit_transitions *self = container_of(
            visitor, struct csp_process_visit_transitions, visit_initial);
    csp_process_visit_afters(csp, self->process, initial, self->wrapped);
}

void
csp_process_visit_transitions(struct csp *csp, struct csp_process *process,
                              struct csp_edge_visitor *visitor)
{
    struct csp_process_visit_transitions self = {
            process, visitor, {csp_process_visit_transitions_visit_initial}};
    csp_process_visit_initials(csp, process, &self.visit_initial);
}

struct csp_process_bfs {
    struct csp_process_set seen;
    struct csp_process_set queue1;
    struct csp_process_set queue2;
    struct csp_process_set *current_queue;
    struct csp_process_set *next_queue;
    struct csp_process_visitor *wrapped;
    struct csp_edge_visitor visit_transition;
};

static void
csp_process_bfs_enqueue(struct csp *csp, struct csp_process_bfs *self,
                        struct csp_process* process)
{
    if (csp_process_set_add(&self->seen, process)) {
        csp_process_set_add(self->next_queue, process);
    }
}

static void
csp_process_bfs_visit_transition(struct csp *csp,
                                 struct csp_edge_visitor *visitor,
                                 const struct csp_event *initial,
                                 struct csp_process *after)
{
    struct csp_process_bfs *self =
            container_of(visitor, struct csp_process_bfs, visit_transition);
    csp_process_bfs_enqueue(csp, self, after);
}

static void
csp_process_bfs_visit_process(struct csp *csp, struct csp_process_bfs *self,
                              struct csp_process *process)
{
    csp_process_visitor_call(csp, self->wrapped, process);
    csp_process_visit_transitions(csp, process, &self->visit_transition);
}

static void
csp_process_bfs_init(struct csp_process_bfs *self,
                     struct csp_process_visitor *wrapped)
{
    csp_process_set_init(&self->seen);
    csp_process_set_init(&self->queue1);
    csp_process_set_init(&self->queue2);
    self->current_queue = &self->queue1;
    self->next_queue = &self->queue2;
    self->wrapped = wrapped;
    self->visit_transition.visit = csp_process_bfs_visit_transition;
}

static void
csp_process_bfs_done(struct csp_process_bfs *self)
{
    csp_process_set_done(&self->seen);
    csp_process_set_done(&self->queue1);
    csp_process_set_done(&self->queue2);
}

void
csp_process_bfs(struct csp *csp, struct csp_process *root,
                struct csp_process_visitor *visitor)
{
    struct csp_process_bfs self;
    csp_process_bfs_init(&self, visitor);
    csp_process_bfs_enqueue(csp, &self, root);
    while (!csp_process_set_empty(self.next_queue)) {
        struct csp_process_set_iterator iter;
        swap(self.current_queue, self.next_queue);
        csp_process_set_clear(self.next_queue);
        csp_process_set_foreach (self.current_queue, &iter) {
            struct csp_process *process = csp_process_set_iterator_get(&iter);
            csp_process_bfs_visit_process(csp, &self, process);
        }
    }
    csp_process_bfs_done(&self);
}

/*------------------------------------------------------------------------------
 * Process sets
 */

void
csp_process_set_init(struct csp_process_set *set)
{
    csp_set_init(&set->set);
}

void
csp_process_set_done(struct csp_process_set *set)
{
    csp_set_done(&set->set, NULL, NULL);
}

bool
csp_process_set_empty(const struct csp_process_set *set)
{
    return csp_set_empty(&set->set);
}

size_t
csp_process_set_size(const struct csp_process_set *set)
{
    return csp_set_size(&set->set);
}

bool
csp_process_set_eq(const struct csp_process_set *set1,
                   const struct csp_process_set *set2)
{
    return csp_set_eq(&set1->set, &set2->set);
}

void
csp_process_set_clear(struct csp_process_set *set)
{
    csp_set_clear(&set->set, NULL, NULL);
}

static int
compare_process_index(const void *p1, const void *p2)
{
    struct csp_process *process1 = *((struct csp_process * const *) p1);
    struct csp_process *process2 = *((struct csp_process * const *) p2);
    if (process1->index < process2->index) {
        return -1;
    } else if (process1->index > process2->index) {
        return 1;
    } else {
        return 0;
    }
}

void
csp_process_set_sort_by_index(const struct csp_process_set *set,
                              size_t *count_out,
                              struct csp_process ***sorted_out)
{
    size_t count = csp_process_set_size(set);
    struct csp_process **sorted = calloc(count, sizeof(struct csp_process *));
    size_t i = 0;
    struct csp_process_set_iterator iter;
    csp_process_set_foreach (set, &iter) {
        struct csp_process *process = csp_process_set_iterator_get(&iter);
        sorted[i++] = process;
    }
    qsort(sorted, count, sizeof(struct csp_process *), compare_process_index);
    *count_out = count;
    *sorted_out = sorted;
}

void
csp_process_set_name(struct csp *csp, const struct csp_process_set *set,
                     struct csp_name_visitor *visitor)
{
    size_t count = 0;
    struct csp_process **sorted = NULL;
    size_t i;
    csp_process_set_sort_by_index(set, &count, &sorted);
    csp_name_visitor_call(csp, visitor, "{");
    for (i = 0; i < count; i++) {
        if (i > 0) {
            csp_name_visitor_call(csp, visitor, ", ");
        }
        csp_process_name(csp, sorted[i], visitor);
    }
    csp_name_visitor_call(csp, visitor, "}");
    free(sorted);
}

void
csp_process_set_nested_name(struct csp *csp, struct csp_process *process,
                            struct csp_process_set *subprocesses,
                            const char *op, struct csp_name_visitor *visitor)
{
    struct csp_process_set_iterator iter;
    size_t i;
    struct csp_process *lhs = NULL;
    struct csp_process *rhs = NULL;

    /* If there aren't exactly two operands, use the replicated syntax. */
    if (csp_process_set_size(subprocesses) != 2) {
        csp_name_visitor_call(csp, visitor, op);
        csp_name_visitor_call(csp, visitor, " ");
        csp_process_set_name(csp, subprocesses, visitor);
        return;
    }

    i = 0;
    csp_process_set_foreach (subprocesses, &iter) {
        if (i++ == 0) {
            lhs = csp_process_set_iterator_get(&iter);
        } else {
            rhs = csp_process_set_iterator_get(&iter);
        }
    }
    if (lhs->index > rhs->index) {
        swap(lhs, rhs);
    }

    csp_process_nested_name(csp, process, lhs, visitor);
    csp_name_visitor_call(csp, visitor, " ");
    csp_name_visitor_call(csp, visitor, op);
    csp_name_visitor_call(csp, visitor, " ");
    csp_process_nested_name(csp, process, rhs, visitor);
}

bool
csp_process_set_add(struct csp_process_set *set, struct csp_process *process)
{
    return csp_set_add(&set->set, (void *) process);
}

bool
csp_process_set_remove(struct csp_process_set *set, struct csp_process *process)
{
    return csp_set_remove(&set->set, (void *) process);
}

bool
csp_process_set_union(struct csp_process_set *set,
                      const struct csp_process_set *other)
{
    return csp_set_union(&set->set, &other->set);
}

void
csp_process_set_get_iterator(const struct csp_process_set *set,
                             struct csp_process_set_iterator *iter)
{
    csp_set_get_iterator(&set->set, &iter->iter);
}

struct csp_process *
csp_process_set_iterator_get(const struct csp_process_set_iterator *iter)
{
    return csp_set_iterator_get(&iter->iter);
}

bool
csp_process_set_iterator_done(struct csp_process_set_iterator *iter)
{
    return csp_set_iterator_done(&iter->iter);
}

void
csp_process_set_iterator_advance(struct csp_process_set_iterator *iter)
{
    csp_set_iterator_advance(&iter->iter);
}

/*------------------------------------------------------------------------------
 * Process bags
 */

void
csp_process_bag_init(struct csp_process_bag *bag)
{
    csp_map_init(&bag->map);
    bag->count = 0;
}

static void
csp_process_bag_free_entry(void *ud, void *entry)
{
    free(entry);
}

void
csp_process_bag_done(struct csp_process_bag *bag)
{
    csp_map_done(&bag->map, csp_process_bag_free_entry, NULL);
}

bool
csp_process_bag_empty(const struct csp_process_bag *bag)
{
    return bag->count == 0;
}

size_t
csp_process_bag_size(const struct csp_process_bag *bag)
{
    return bag->count;
}

static bool
csp_process_bag_entry_eq(void *ud, const void *vcount1, const void *vcount2)
{
    uintptr_t count1 = (uintptr_t) vcount1;
    uintptr_t count2 = (uintptr_t) vcount2;
    return count1 == count2;
}

bool
csp_process_bag_eq(const struct csp_process_bag *bag1,
                   const struct csp_process_bag *bag2)
{
    return csp_map_eq(&bag1->map, &bag2->map, csp_process_bag_entry_eq, NULL);
}

void
csp_process_bag_clear(struct csp_process_bag *bag)
{
    csp_process_bag_done(bag);
    csp_process_bag_init(bag);
}

void
csp_process_bag_sort_by_index(const struct csp_process_bag *bag,
                              size_t *count_out,
                              struct csp_process ***sorted_out)
{
    size_t count = csp_process_bag_size(bag);
    struct csp_process **sorted = calloc(count, sizeof(struct csp_process *));
    size_t i = 0;
    struct csp_process_bag_iterator iter;
    csp_process_bag_foreach (bag, &iter) {
        struct csp_process *process = csp_process_bag_iterator_get(&iter);
        size_t count = csp_process_bag_iterator_get_count(&iter);
        size_t j;
        for (j = 0; j < count; j++) {
            sorted[i++] = process;
        }
    }
    qsort(sorted, count, sizeof(struct csp_process *), compare_process_index);
    *count_out = count;
    *sorted_out = sorted;
}

void
csp_process_bag_name(struct csp *csp, const struct csp_process_bag *bag,
                     struct csp_name_visitor *visitor)
{
    size_t count = 0;
    struct csp_process **sorted = NULL;
    size_t i;
    csp_process_bag_sort_by_index(bag, &count, &sorted);
    csp_name_visitor_call(csp, visitor, "{");
    for (i = 0; i < count; i++) {
        if (i > 0) {
            csp_name_visitor_call(csp, visitor, ", ");
        }
        csp_process_name(csp, sorted[i], visitor);
    }
    csp_name_visitor_call(csp, visitor, "}");
    free(sorted);
}

void
csp_process_bag_nested_name(struct csp *csp, struct csp_process *process,
                            struct csp_process_bag *subprocesses,
                            const char *op, struct csp_name_visitor *visitor)
{
    struct csp_process_bag_iterator iter;
    size_t i;
    struct csp_process *lhs = NULL;
    struct csp_process *rhs = NULL;

    /* If there aren't exactly two operands, use the replicated syntax. */
    if (csp_process_bag_size(subprocesses) != 2) {
        csp_name_visitor_call(csp, visitor, op);
        csp_name_visitor_call(csp, visitor, " ");
        csp_process_bag_name(csp, subprocesses, visitor);
        return;
    }

    i = 0;
    csp_process_bag_foreach (subprocesses, &iter) {
        struct csp_process *process = csp_process_bag_iterator_get(&iter);
        size_t count = csp_process_bag_iterator_get_count(&iter);
        size_t j;
        for (j = 0; j < count; j++) {
            if (i++ == 0) {
                lhs = process;
            } else {
                rhs = process;
            }
        }
    }
    if (lhs->index > rhs->index) {
        swap(lhs, rhs);
    }

    csp_process_nested_name(csp, process, lhs, visitor);
    csp_name_visitor_call(csp, visitor, " ");
    csp_name_visitor_call(csp, visitor, op);
    csp_name_visitor_call(csp, visitor, " ");
    csp_process_nested_name(csp, process, rhs, visitor);
}

void
csp_process_bag_add(struct csp_process_bag *bag, struct csp_process *process)
{
    uintptr_t *count = csp_map_get(&bag->map, (uintptr_t) process);
    bag->count++;
    if (unlikely(count == NULL)) {
        count = malloc(sizeof(uintptr_t));
        *count = 1;
        csp_map_insert(&bag->map, (uintptr_t) process, count);
    } else {
        (*count)++;
    }
}

void
csp_process_bag_remove(struct csp_process_bag *bag, struct csp_process *process)
{
    uintptr_t *count = csp_map_get(&bag->map, (uintptr_t) process);
    assert(count != NULL && *count > 0);
    bag->count--;
    if (*count == 1) {
        csp_map_remove(&bag->map, (uintptr_t) process,
                       csp_process_bag_free_entry, NULL);
    } else {
        (*count)--;
    }
}

void
csp_process_bag_union(struct csp_process_bag *bag,
                      const struct csp_process_bag *other)
{
    struct csp_process_bag_iterator iter;
    bag->count += other->count;
    csp_process_bag_foreach (other, &iter) {
        struct csp_process *process = csp_process_bag_iterator_get(&iter);
        size_t count = csp_process_bag_iterator_get_count(&iter);
        uintptr_t *our_count = csp_map_get(&bag->map, (uintptr_t) process);
        if (unlikely(our_count == NULL)) {
            our_count = malloc(sizeof(uintptr_t));
            *our_count = count;
            csp_map_insert(&bag->map, (uintptr_t) process, our_count);
        } else {
            *our_count += count;
        }
    }
}

void
csp_process_bag_get_iterator(const struct csp_process_bag *bag,
                             struct csp_process_bag_iterator *iter)
{
    csp_map_get_iterator(&bag->map, &iter->iter);
}

struct csp_process *
csp_process_bag_iterator_get(const struct csp_process_bag_iterator *iter)
{
    return (void *) csp_map_iterator_get_key(&iter->iter);
}

size_t
csp_process_bag_iterator_get_count(const struct csp_process_bag_iterator *iter)
{
    return *((uintptr_t *) csp_map_iterator_get_value(&iter->iter));
}

bool
csp_process_bag_iterator_done(struct csp_process_bag_iterator *iter)
{
    return csp_map_iterator_done(&iter->iter);
}

void
csp_process_bag_iterator_advance(struct csp_process_bag_iterator *iter)
{
    csp_map_iterator_advance(&iter->iter);
}
