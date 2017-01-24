/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "event.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/container_of/container_of.h"
#include "ccan/hash/hash.h"
#include "ccan/likely/likely.h"
#include "basics.h"
#include "map.h"
#include "set.h"

/*------------------------------------------------------------------------------
 * Events
 */

struct csp_event {
    csp_id id;
    const char *name;
};

static struct csp_event *
csp_event_new(csp_id id, const char *name, size_t name_length)
{
    struct csp_event *event =
            malloc(sizeof(struct csp_event) + name_length + 1);
    char *name_copy;
    assert(event != NULL);
    name_copy = (void *) (event + 1);
    memcpy(name_copy, name, name_length);
    name_copy[name_length] = '\0';
    event->id = id;
    event->name = name_copy;
    return event;
}

static void
csp_event_free(struct csp_event *event)
{
    free(event);
}

/*------------------------------------------------------------------------------
 * Event map
 */

struct csp_event_map {
    struct csp_map map;
};

static void
csp_event_map_init(struct csp_event_map *map)
{
    csp_map_init(&map->map);
}

static void
csp_event_map_free_entry(void *ud, void *entry)
{
    struct csp_event *event = entry;
    csp_event_free(event);
}

static void
csp_event_map_done(struct csp_event_map *map)
{
    csp_map_done(&map->map, csp_event_map_free_entry, NULL);
}

static const struct csp_event *
csp_event_map_get(struct csp_event_map *map, csp_id id)
{
    return csp_map_get(&map->map, id);
}

static void
csp_event_map_insert(struct csp_event_map *map, csp_id id,
                     const struct csp_event *event)
{
    csp_map_insert(&map->map, id, (void *) event);
}

static struct csp_event_map events;

static void
free_event_map(void)
{
    csp_event_map_done(&events);
}

static struct csp_event_map *
get_event_map(void)
{
    static bool initialized = false;
    if (unlikely(!initialized)) {
        csp_event_map_init(&events);
        atexit(free_event_map);
        initialized = true;
    }
    return &events;
}

/*------------------------------------------------------------------------------
 * Public API
 */

static uint64_t
hash_sized_name(const char *name, size_t name_length)
{
    return hash64_any(name, name_length, 0);
}

const struct csp_event *
csp_event_get(const char *name)
{
    return csp_event_get_sized(name, strlen(name));
}

const struct csp_event *
csp_event_get_sized(const char *name, size_t name_length)
{
    struct csp_event_map *map = get_event_map();
    csp_id event_id = hash_sized_name(name, name_length);
    const struct csp_event *event = csp_event_map_get(map, event_id);
    if (unlikely(event == NULL)) {
        event = csp_event_new(event_id, name, name_length);
        csp_event_map_insert(map, event_id, event);
    }
    return event;
}

csp_id
csp_event_id(const struct csp_event *event)
{
    return event->id;
}

const char *
csp_event_name(const struct csp_event *event)
{
    return event->name;
}

/*------------------------------------------------------------------------------
 * Predefined events
 */

const struct csp_event *
csp_tau(void)
{
    static const struct csp_event *tau = NULL;
    if (unlikely(tau == NULL)) {
        tau = csp_event_get("τ");
    }
    return tau;
}

const struct csp_event *
csp_tick(void)
{
    static const struct csp_event *tick = NULL;
    if (unlikely(tick == NULL)) {
        tick = csp_event_get("✔");
    }
    return tick;
}

/*------------------------------------------------------------------------------
 * Event sets
 */

#define CSP_EVENT_SET_INITIAL_HASH UINT64_C(0xbe70ef9046515956) /* random */

void
csp_event_set_init(struct csp_event_set *set)
{
    csp_set_init(&set->set);
}

void
csp_event_set_done(struct csp_event_set *set)
{
    csp_set_done(&set->set, NULL, NULL);
}

uint64_t
csp_event_set_hash(const struct csp_event_set *set)
{
    return csp_set_hash(&set->set, CSP_EVENT_SET_INITIAL_HASH);
}

bool
csp_event_set_empty(const struct csp_event_set *set)
{
    return csp_set_empty(&set->set);
}

size_t
csp_event_set_size(const struct csp_event_set *set)
{
    return csp_set_size(&set->set);
}

bool
csp_event_set_eq(const struct csp_event_set *set1,
                 const struct csp_event_set *set2)
{
    return csp_set_eq(&set1->set, &set2->set);
}

bool
csp_event_set_subseteq(const struct csp_event_set *set1,
                       const struct csp_event_set *set2)
{
    return csp_set_subseteq(&set1->set, &set2->set);
}

void
csp_event_set_clear(struct csp_event_set *set)
{
    csp_set_clear(&set->set, NULL, NULL);
}

bool
csp_event_set_add(struct csp_event_set *set, const struct csp_event *event)
{
    return csp_set_add(&set->set, (void *) event);
}

bool
csp_event_set_remove(struct csp_event_set *set, const struct csp_event *event)
{
    return csp_set_remove(&set->set, (void *) event);
}

bool
csp_event_set_union(struct csp_event_set *set,
                    const struct csp_event_set *other)
{
    return csp_set_union(&set->set, &other->set);
}

void
csp_event_set_visit(struct csp *csp, const struct csp_event_set *set,
                    struct csp_event_visitor *visitor)
{
    struct csp_event_set_iterator iter;
    csp_event_set_foreach (set, &iter) {
        const struct csp_event *event = csp_event_set_iterator_get(&iter);
        csp_event_visitor_call(csp, visitor, event);
    }
}

void
csp_event_set_get_iterator(const struct csp_event_set *set,
                           struct csp_event_set_iterator *iter)
{
    csp_set_get_iterator(&set->set, &iter->iter);
}

const struct csp_event *
csp_event_set_iterator_get(const struct csp_event_set_iterator *iter)
{
    return csp_set_iterator_get(&iter->iter);
}

bool
csp_event_set_iterator_done(struct csp_event_set_iterator *iter)
{
    return csp_set_iterator_done(&iter->iter);
}

void
csp_event_set_iterator_advance(struct csp_event_set_iterator *iter)
{
    csp_set_iterator_advance(&iter->iter);
}

/*------------------------------------------------------------------------------
 * Event visitors
 */

void
csp_event_visitor_call(struct csp *csp, struct csp_event_visitor *visitor,
                       const struct csp_event *event)
{
    visitor->visit(csp, visitor, event);
}

static void
csp_any_events_visit(struct csp *csp, struct csp_event_visitor *visitor,
                     const struct csp_event *event)
{
    struct csp_any_events *self =
            container_of(visitor, struct csp_any_events, visitor);
    self->has_events = true;
}

struct csp_any_events
csp_any_events(void)
{
    struct csp_any_events self = {{csp_any_events_visit}, false};
    return self;
}

static void
csp_contains_event_visit(struct csp *csp, struct csp_event_visitor *visitor,
                         const struct csp_event *event)
{
    struct csp_contains_event *self =
            container_of(visitor, struct csp_contains_event, visitor);
    if (event == self->event) {
        self->is_present = true;
    }
}

struct csp_contains_event
csp_contains_event(const struct csp_event *event)
{
    struct csp_contains_event self = {{csp_contains_event_visit}, event, false};
    return self;
}

static void
csp_collect_events_visit(struct csp *csp, struct csp_event_visitor *visitor,
                         const struct csp_event *event)
{
    struct csp_collect_events *self =
            container_of(visitor, struct csp_collect_events, visitor);
    csp_event_set_add(self->set, event);
}

struct csp_collect_events
csp_collect_events(struct csp_event_set *set)
{
    struct csp_collect_events self = {{csp_collect_events_visit}, set};
    return self;
}

static void
csp_ignore_event_visit(struct csp *csp, struct csp_event_visitor *visitor,
                       const struct csp_event *event)
{
    struct csp_ignore_event *self =
            container_of(visitor, struct csp_ignore_event, visitor);
    if (event != self->event) {
        csp_event_visitor_call(csp, self->wrapped, event);
    }
}

struct csp_ignore_event
csp_ignore_event(struct csp_event_visitor *wrapped,
                 const struct csp_event *event)
{
    struct csp_ignore_event self = {{csp_ignore_event_visit}, wrapped, event};
    return self;
}
