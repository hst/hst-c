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
#include "event.h"
#include "map.h"
#include "process.h"

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
csp_stop_name(struct csp *csp, struct csp_process *process,
              struct csp_name_visitor *visitor)
{
    csp_name_visitor_call(csp, visitor, "STOP");
}

static void
csp_stop_initials(struct csp *csp, struct csp_process *process,
                  struct csp_event_visitor *visitor)
{
}

static void
csp_stop_afters(struct csp *csp, struct csp_process *process,
                const struct csp_event *initial,
                struct csp_edge_visitor *visitor)
{
}

static void
csp_stop_free(struct csp *csp, struct csp_process *process)
{
}

static const struct csp_process_iface csp_stop_iface = {
        1, csp_stop_name, csp_stop_initials, csp_stop_afters, csp_stop_free};

static struct csp_process *
csp_stop(void)
{
    static struct csp_process stop;
    static bool initialized = false;
    if (unlikely(!initialized)) {
        stop.id = hash_name("STOP");
        stop.iface = &csp_stop_iface;
    }
    return &stop;
}

static void
csp_skip_name(struct csp *csp, struct csp_process *process,
              struct csp_name_visitor *visitor)
{
    csp_name_visitor_call(csp, visitor, "SKIP");
}

static void
csp_skip_initials(struct csp *csp, struct csp_process *process,
                  struct csp_event_visitor *visitor)
{
    csp_event_visitor_call(csp, visitor, csp->tick);
}

static void
csp_skip_afters(struct csp *csp, struct csp_process *process,
                const struct csp_event *initial,
                struct csp_edge_visitor *visitor)
{
    if (initial == csp->tick) {
        csp_edge_visitor_call(csp, visitor, initial, csp->stop);
    }
}

static void
csp_skip_free(struct csp *csp, struct csp_process *process)
{
}

static const struct csp_process_iface csp_skip_iface = {
        1, csp_skip_name, csp_skip_initials, csp_skip_afters, csp_skip_free};

static struct csp_process *
csp_skip(void)
{
    static struct csp_process skip;
    static bool initialized = false;
    if (unlikely(!initialized)) {
        skip.id = hash_name("skip");
        skip.iface = &csp_skip_iface;
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
    size_t process_count;
    struct csp_id_process_map processes;
};

struct csp *
csp_new(void)
{
    struct csp_priv *csp = malloc(sizeof(struct csp_priv));
    if (unlikely(csp == NULL)) {
        return NULL;
    }
    csp_id_process_map_init(&csp->processes);
    csp->process_count = 0;
    csp->next_recursion_scope_id = 0;
    csp->public.tau = csp_tau();
    csp->public.tick = csp_tick();
    csp->public.stop = csp_stop();
    csp_register_process(&csp->public, csp->public.stop);
    csp->public.skip = csp_skip();
    csp_register_process(&csp->public, csp->public.skip);
    return &csp->public;
}

void
csp_free(struct csp *pcsp)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    csp_id_process_map_done(&csp->public, &csp->processes);
    free(csp);
}

void
csp_register_process(struct csp *pcsp, struct csp_process *process)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process **entry =
            csp_id_process_map_at(&csp->processes, process->id);
    assert(*entry == NULL);
    *entry = process;
    process->index = csp->process_count++;
}

struct csp_process *
csp_get_process(struct csp *pcsp, csp_id process_id)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    return csp_id_process_map_get(&csp->processes, process_id);
}

struct csp_process *
csp_require_process(struct csp *csp, csp_id id)
{
    struct csp_process *process = csp_get_process(csp, id);
    assert(process != NULL);
    return process;
}

csp_id
csp_id_start(struct csp_id_scope *scope)
{
    return hash64_any(&scope, sizeof(struct csp_id_scope *), 0);
}

csp_id
csp_id_add_event(csp_id id, const struct csp_event *event)
{
    return hash64_any(&event, sizeof(const struct csp_event *), id);
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

csp_id
csp_id_add_process(csp_id id, struct csp_process *process)
{
    return csp_id_add_id(id, process->id);
}

csp_id
csp_id_add_process_set(csp_id id, const struct csp_process_set *set)
{
    struct csp_process_set_iterator iter;
    csp_process_set_foreach(set, &iter) {
        id = csp_id_add_process(id, csp_process_set_iterator_get(&iter));
    }
    return id;
}
