/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PROCESS_H
#define HST_PROCESS_H

#include <stdio.h>
#include <stdlib.h>

#include "basics.h"
#include "event.h"
#include "map.h"
#include "set.h"

struct csp;
struct csp_process;

/*------------------------------------------------------------------------------
 * Edge visitors
 */

struct csp_edge_visitor {
    void (*visit)(struct csp *csp, struct csp_edge_visitor *visitor,
                  const struct csp_event *event, struct csp_process *after);
};

void
csp_edge_visitor_call(struct csp *csp, struct csp_edge_visitor *visitor,
                      const struct csp_event *event, struct csp_process *after);

struct csp_any_edges {
    struct csp_edge_visitor visitor;
    bool has_edges;
};

struct csp_any_edges
csp_any_edges(void);

struct csp_collect_afters {
    struct csp_edge_visitor visitor;
    struct csp_process_set *set;
};

struct csp_collect_afters
csp_collect_afters(struct csp_process_set *set);

/*------------------------------------------------------------------------------
 * Process visitors
 */

struct csp_process_visitor {
    int (*visit)(struct csp *csp, struct csp_process_visitor *visitor,
                 struct csp_process *process);
};

int
csp_process_visitor_call(struct csp *csp, struct csp_process_visitor *visitor,
                         struct csp_process *process);

struct csp_collect_processes {
    struct csp_process_visitor visitor;
    struct csp_process_set *set;
};

struct csp_collect_processes
csp_collect_processes(struct csp_process_set *set);

/*------------------------------------------------------------------------------
 * Name visitor
 */

struct csp_name_visitor {
    void (*visit)(struct csp *csp, struct csp_name_visitor *visitor,
                  const char *str, size_t length);
};

void
csp_name_visitor_call(struct csp *csp, struct csp_name_visitor *visitor,
                      const char *str);

void
csp_name_visitor_call_sized(struct csp *csp, struct csp_name_visitor *visitor,
                            const char *str, size_t length);

struct csp_print_name {
    struct csp_name_visitor visitor;
    FILE *out;
};

struct csp_print_name
csp_print_name(FILE *out);

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process_iface {
    unsigned int precedence;

    void (*name)(struct csp *csp, struct csp_process *process,
                 struct csp_name_visitor *visitor);

    void (*initials)(struct csp *csp, struct csp_process *process,
                     struct csp_event_visitor *visitor);

    void (*afters)(struct csp *csp, struct csp_process *process,
                   const struct csp_event *initial,
                   struct csp_edge_visitor *visitor);

    void (*free)(struct csp *csp, struct csp_process *process);
};

struct csp_process {
    csp_id id;
    const struct csp_process_iface *iface;
    size_t index;
};

void
csp_process_free(struct csp *csp, struct csp_process *process);

void
csp_process_name(struct csp *csp, struct csp_process *process,
                 struct csp_name_visitor *visitor);

/* Renders the name of `subprocess` as part of the name of `process`.  The
 * precedence values of the two processes will determine whether we need to wrap
 * `subprocess` in parentheses or not. */
void
csp_process_nested_name(struct csp *csp, struct csp_process *process,
                        struct csp_process *subprocess,
                        struct csp_name_visitor *visitor);

void
csp_process_visit_initials(struct csp *csp, struct csp_process *process,
                           struct csp_event_visitor *visitor);

void
csp_process_visit_afters(struct csp *csp, struct csp_process *process,
                         const struct csp_event *initial,
                         struct csp_edge_visitor *visitor);

void
csp_process_visit_transitions(struct csp *csp, struct csp_process *process,
                              struct csp_edge_visitor *visitor);

#define CSP_PROCESS_BFS_CONTINUE 0
#define CSP_PROCESS_BFS_ABORT 1
#define CSP_PROCESS_BFS_PRUNE 2

/* If your callback ever returns `PRUNE`, then we will not follow any outgoing
 * transitions from the corresponding process.  If your callback ever returns
 * `ABORT`, we will immediately abort the entire BFS. */
void
csp_process_bfs(struct csp *csp, struct csp_process *process,
                struct csp_process_visitor *visitor);

/*------------------------------------------------------------------------------
 * Process sets
 */

struct csp_process_set {
    struct csp_set set;
};

struct csp_process_set *
csp_process_set_new(void);

void
csp_process_set_init(struct csp_process_set *set);

void
csp_process_set_done(struct csp_process_set *set);

void
csp_process_set_free(struct csp_process_set *set);

/* We'll make sure this gets freed. */
const struct csp_process_set *
csp_process_set_new_empty(void);

bool
csp_process_set_empty(const struct csp_process_set *set);

size_t
csp_process_set_size(const struct csp_process_set *set);

bool
csp_process_set_eq(const struct csp_process_set *set1,
                   const struct csp_process_set *set2);

void
csp_process_set_clear(struct csp_process_set *set);

/* Fills in `count` with the number of processes in `set`, and `processes` with
 * an array of those processes, sorted by their `index` values.  You must free
 * `processes` when you're done with it (using `free(3)`). */
void
csp_process_set_sort_by_index(const struct csp_process_set *set, size_t *count,
                              struct csp_process ***processes);

/* Renders the name of each process in a set, in some braces to show that it's a
 * set. */
void
csp_process_set_name(struct csp *csp, const struct csp_process_set *set,
                     struct csp_name_visitor *visitor);

/* Renders a process whose operator can appear infix between two subprocesses,
 * or prefix before a set of subprocesses.  Chooses which version to render
 * based on the size of `subprocesses`. */
void
csp_process_set_nested_name(struct csp *csp, struct csp_process *process,
                            struct csp_process_set *subprocesses,
                            const char *op, struct csp_name_visitor *visitor);

/* Add a single process to a set.  Return whether the process is new (i.e., it
 * wasn't already in `set`.) */
bool
csp_process_set_add(struct csp_process_set *set, struct csp_process *process);

/* Remove a single process from a set.  Returns whether that process was in the
 * set or not. */
bool
csp_process_set_remove(struct csp_process_set *set,
                       struct csp_process *process);

/* Add the contents of an existing set to a set.  Returns true if any new
 * elements were added. */
bool
csp_process_set_union(struct csp_process_set *set,
                      const struct csp_process_set *other);

struct csp_process_set_iterator {
    struct csp_set_iterator iter;
};

void
csp_process_set_get_iterator(const struct csp_process_set *set,
                             struct csp_process_set_iterator *iter);

struct csp_process *
csp_process_set_iterator_get(const struct csp_process_set_iterator *iter);

bool
csp_process_set_iterator_done(struct csp_process_set_iterator *iter);

void
csp_process_set_iterator_advance(struct csp_process_set_iterator *iter);

#define csp_process_set_foreach(set, iter)            \
    for (csp_process_set_get_iterator((set), (iter)); \
         !csp_process_set_iterator_done((iter));      \
         csp_process_set_iterator_advance((iter)))

/*------------------------------------------------------------------------------
 * Process bags
 */

struct csp_process_bag {
    struct csp_map map;
    size_t count;
};

void
csp_process_bag_init(struct csp_process_bag *bag);

void
csp_process_bag_done(struct csp_process_bag *bag);

bool
csp_process_bag_empty(const struct csp_process_bag *bag);

size_t
csp_process_bag_size(const struct csp_process_bag *bag);

bool
csp_process_bag_eq(const struct csp_process_bag *bag1,
                   const struct csp_process_bag *bag2);

void
csp_process_bag_clear(struct csp_process_bag *bag);

/* Fills in `count` with the number of processes in `bag`, and `processes` with
 * an array of those processes, sorted by their `index` values.  You must free
 * `processes` when you're done with it (using `free(3)`). */
void
csp_process_bag_sort_by_index(const struct csp_process_bag *bag, size_t *count,
                              struct csp_process ***processes);

/* Renders the name of each process in a bag, in some braces to show that it's a
 * bag. */
void
csp_process_bag_name(struct csp *csp, const struct csp_process_bag *bag,
                     struct csp_name_visitor *visitor);

/* Renders a process whose operator can appear infix between two subprocesses,
 * or prefix before a bag of subprocesses.  Chooses which version to render
 * based on the size of `subprocesses`. */
void
csp_process_bag_nested_name(struct csp *csp, struct csp_process *process,
                            struct csp_process_bag *subprocesses,
                            const char *op, struct csp_name_visitor *visitor);

/* Add a single process to a bag. */
void
csp_process_bag_add(struct csp_process_bag *bag, struct csp_process *process);

/* Remove a single process from a bag.  `process` must be in the bag. */
void
csp_process_bag_remove(struct csp_process_bag *bag,
                       struct csp_process *process);

/* Add the contents of an existing bag to a bag. */
void
csp_process_bag_union(struct csp_process_bag *bag,
                      const struct csp_process_bag *other);

struct csp_process_bag_iterator {
    struct csp_map_iterator iter;
};

void
csp_process_bag_get_iterator(const struct csp_process_bag *bag,
                             struct csp_process_bag_iterator *iter);

struct csp_process *
csp_process_bag_iterator_get(const struct csp_process_bag_iterator *iter);

size_t
csp_process_bag_iterator_get_count(const struct csp_process_bag_iterator *iter);

bool
csp_process_bag_iterator_done(struct csp_process_bag_iterator *iter);

void
csp_process_bag_iterator_advance(struct csp_process_bag_iterator *iter);

#define csp_process_bag_foreach(bag, iter)            \
    for (csp_process_bag_get_iterator((bag), (iter)); \
         !csp_process_bag_iterator_done((iter));      \
         csp_process_bag_iterator_advance((iter)))

#endif /* HST_PROCESS_H */
