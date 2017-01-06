/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "environment.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/compiler/compiler.h"
#include "ccan/container_of/container_of.h"
#include "ccan/hash/hash.h"
#include "ccan/likely/likely.h"
#include "map.h"
#include "process.h"
#include "string-map.h"

static uint64_t
hash_sized_name(const char *name, size_t name_length)
{
    return hash64_any(name, name_length, 0);
}

static uint64_t
hash_name(const char *name)
{
    return hash_sized_name(name, strlen(name));
}

/*------------------------------------------------------------------------------
 * Predefined processes
 */

static void
csp_stop_initials(struct csp *csp, struct csp_process *process,
                  struct csp_id_set *set)
{
}

static void
csp_stop_afters(struct csp *csp, struct csp_process *process, csp_id initial,
                struct csp_id_set *set)
{
}

static void
csp_stop_free(struct csp *csp, struct csp_process *process)
{
}

static struct csp_process *
csp_stop(void)
{
    static struct csp_process stop;
    static bool initialized = false;
    if (unlikely(!initialized)) {
        stop.id = hash_name("STOP");
        stop.initials = csp_stop_initials;
        stop.afters = csp_stop_afters;
        stop.free = csp_stop_free;
        initialized = true;
    }
    return &stop;
}

static void
csp_skip_initials(struct csp *csp, struct csp_process *process,
                  struct csp_id_set *set)
{
    csp_id_set_add(set, csp->tick);
}

static void
csp_skip_afters(struct csp *csp, struct csp_process *process, csp_id initial,
                struct csp_id_set *set)
{
    if (initial == csp->tick) {
        csp_id_set_add(set, csp->stop);
    }
}

static void
csp_skip_free(struct csp *csp, struct csp_process *process)
{
}

static struct csp_process *
csp_skip(void)
{
    static struct csp_process skip;
    static bool initialized = false;
    if (unlikely(!initialized)) {
        skip.id = hash_name("skip");
        skip.initials = csp_skip_initials;
        skip.afters = csp_skip_afters;
        skip.free = csp_skip_free;
        initialized = true;
    }
    return &skip;
}

/*------------------------------------------------------------------------------
 * ID→process maps
 */

struct csp_id_process_map {
    struct csp_map map;
};

static void
csp_id_process_map_init(struct csp_id_process_map *map)
{
    csp_map_init(&map->map);
}

static void
csp_id_process_map_free_entry(void *ud, void *entry)
{
    struct csp *csp = ud;
    struct csp_process *process = entry;
    csp_process_free(csp, process);
}

void
csp_id_process_map_done(struct csp *csp, struct csp_id_process_map *map)
{
    csp_map_done(&map->map, csp_id_process_map_free_entry, csp);
}

struct csp_process *
csp_id_process_map_get(const struct csp_id_process_map *map, csp_id id)
{
    return csp_map_get(&map->map, id);
}

struct csp_process **
csp_id_process_map_at(struct csp_id_process_map *map, csp_id id)
{
    return (struct csp_process **) csp_map_at(&map->map, id);
}

/*------------------------------------------------------------------------------
 * Environment
 */

struct csp_priv {
    struct csp public;
    csp_id next_recursion_scope_id;
    struct csp_string_map events;
    struct csp_id_process_map processes;
    struct csp_process *stop;
    struct csp_process *skip;
};

struct csp *
csp_new(void)
{
    static const char *const TAU = "τ";
    static const char *const TICK = "✔";
    struct csp_priv *csp = malloc(sizeof(struct csp_priv));
    if (unlikely(csp == NULL)) {
        return NULL;
    }
    csp_string_map_init(&csp->events);
    csp_id_process_map_init(&csp->processes);
    csp->next_recursion_scope_id = 0;
    csp->public.tau = csp_get_event_id(&csp->public, TAU);
    csp->public.tick = csp_get_event_id(&csp->public, TICK);
    csp->stop = csp_stop();
    csp_register_process(&csp->public, csp->stop);
    csp->public.stop = csp->stop->id;
    csp->skip = csp_skip();
    csp_register_process(&csp->public, csp->skip);
    csp->public.skip = csp->skip->id;
    return &csp->public;
}

void
csp_free(struct csp *pcsp)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    csp_string_map_done(&csp->events);
    csp_id_process_map_done(&csp->public, &csp->processes);
    free(csp);
}

csp_id
csp_get_event_id(struct csp *pcsp, const char *name)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    csp_id event = hash_name(name);
    csp_string_map_insert(&csp->events, event, name);
    return event;
}

csp_id
csp_get_sized_event_id(struct csp *pcsp, const char *name, size_t name_length)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    csp_id event = hash_sized_name(name, name_length);
    csp_string_map_insert_sized(&csp->events, event, name, name_length);
    return event;
}

const char *
csp_get_event_name(struct csp *pcsp, csp_id event)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    return csp_string_map_get(&csp->events, event);
}

void
csp_register_process(struct csp *pcsp, struct csp_process *process)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process **entry =
            csp_id_process_map_at(&csp->processes, process->id);
    assert(*entry == NULL);
    *entry = process;
}

struct csp_process *
csp_get_process(struct csp *pcsp, csp_id process_id)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    return csp_id_process_map_get(&csp->processes, process_id);
}

void
csp_build_process_initials(struct csp *csp, csp_id process_id,
                           struct csp_id_set *set)
{
    struct csp_process *process = csp_get_process(csp, process_id);
    assert(process != NULL);
    csp_process_build_initials(csp, process, set);
}

void
csp_build_process_afters(struct csp *csp, csp_id process_id, csp_id initial,
                         struct csp_id_set *set)
{
    struct csp_process *process = csp_get_process(csp, process_id);
    assert(process != NULL);
    csp_process_build_afters(csp, process, initial, set);
}

csp_id
csp_id_start(struct csp_id_scope *scope)
{
    return hash64_any(&scope, sizeof(struct csp_id_scope *), 0);
}

csp_id
csp_id_add_id(csp_id id, csp_id id_to_add)
{
    return hash64_any(&id_to_add, sizeof(csp_id), id);
}

csp_id
csp_id_add_id_set(csp_id id, const struct csp_id_set *set)
{
    struct csp_id_set_iterator iter;
    csp_id_set_foreach(set, &iter) {
        id = csp_id_add_id(id, csp_id_set_iterator_get(&iter));
    }
    return id;
}

csp_id
csp_id_add_name(csp_id id, const char *name)
{
    return hash64_any(name, strlen(name), id);
}

csp_id
csp_id_add_name_sized(csp_id id, const char *name, size_t name_length)
{
    return hash64_any(name, name_length, id);
}
