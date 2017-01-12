/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_EVENT_H
#define HST_EVENT_H

#include <stdlib.h>

#include "ccan/compiler/compiler.h"
#include "basics.h"
#include "set.h"

/*------------------------------------------------------------------------------
 * Events
 */

struct csp_event;

CONST_FUNCTION
const struct csp_event *
csp_tau(void);

CONST_FUNCTION
const struct csp_event *
csp_tick(void);

/* Return the ID of the event with the given name.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
PURE_FUNCTION
const struct csp_event *
csp_event_get(const char *name);

PURE_FUNCTION
const struct csp_event *
csp_event_get_sized(const char *name, size_t name_length);

PURE_FUNCTION
csp_id
csp_event_id(const struct csp_event *event);

PURE_FUNCTION
const char *
csp_event_name(const struct csp_event *event);

/*------------------------------------------------------------------------------
 * Event sets
 */

struct csp_event_set {
    struct csp_set set;
};

void
csp_event_set_init(struct csp_event_set *set);

void
csp_event_set_done(struct csp_event_set *set);

uint64_t
csp_event_set_hash(const struct csp_event_set *set);

bool
csp_event_set_empty(const struct csp_event_set *set);

size_t
csp_event_set_size(const struct csp_event_set *set);

bool
csp_event_set_eq(const struct csp_event_set *set1,
                 const struct csp_event_set *set2);

/* Return whether set1 ⊆ set2 */
bool
csp_event_set_subseteq(const struct csp_event_set *set1,
                       const struct csp_event_set *set2);

void
csp_event_set_clear(struct csp_event_set *set);

/* Add a single event to a set.  Return whether the event is new (i.e., it
 * wasn't already in `set`.) */
bool
csp_event_set_add(struct csp_event_set *set, const struct csp_event *event);

/* Remove a single event from a set.  Returns whether that event was in the set
 * or not. */
bool
csp_event_set_remove(struct csp_event_set *set, const struct csp_event *event);

/* Add the contents of an existing set to a set.  Returns true if any new
 * elements were added. */
bool
csp_event_set_union(struct csp_event_set *set,
                    const struct csp_event_set *other);

struct csp_event_set_iterator {
    struct csp_set_iterator iter;
};

void
csp_event_set_get_iterator(const struct csp_event_set *set,
                           struct csp_event_set_iterator *iter);

const struct csp_event *
csp_event_set_iterator_get(const struct csp_event_set_iterator *iter);

bool
csp_event_set_iterator_done(struct csp_event_set_iterator *iter);

void
csp_event_set_iterator_advance(struct csp_event_set_iterator *iter);

#define csp_event_set_foreach(set, iter)            \
    for (csp_event_set_get_iterator((set), (iter)); \
         !csp_event_set_iterator_done((iter));      \
         csp_event_set_iterator_advance((iter)))

#endif /* HST_EVENT_H */
