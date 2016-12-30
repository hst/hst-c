/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
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
csp_stop_initials(struct csp *csp, struct csp_id_set *set, void *ud)
{
}

static void
csp_stop_afters(struct csp *csp, csp_id initial, struct csp_id_set *set,
                void *ud)
{
}

static csp_id
csp_stop_get_id(struct csp *csp, const void *temp_ud)
{
    return hash_name("STOP");
}

static size_t
csp_stop_ud_size(struct csp *csp, const void *temp_ud)
{
    return 0;
}

static void
csp_stop_init(struct csp *csp, void *ud, const void *temp_ud)
{
    /* nothing to do */
}

static void
csp_stop_done(struct csp *csp, void *ud)
{
    /* nothing to do */
}

const struct csp_process_iface csp_stop_iface = {
        &csp_stop_initials, &csp_stop_afters, &csp_stop_get_id,
        &csp_stop_ud_size,  &csp_stop_init,   &csp_stop_done};

static void
csp_skip_initials(struct csp *csp, struct csp_id_set *set, void *ud)
{
    csp_id_set_add(set, csp->tick);
}

static void
csp_skip_afters(struct csp *csp, csp_id initial, struct csp_id_set *set,
                void *ud)
{
    if (initial == csp->tick) {
        csp_id_set_add(set, csp->stop);
    }
}

static csp_id
csp_skip_get_id(struct csp *csp, const void *temp_ud)
{
    return hash_name("SKIP");
}

static size_t
csp_skip_ud_size(struct csp *csp, const void *temp_ud)
{
    return 0;
}

static void
csp_skip_init(struct csp *csp, void *ud, const void *temp_ud)
{
    /* nothing to do */
}

static void
csp_skip_done(struct csp *csp, void *ud)
{
    /* nothing to do */
}

const struct csp_process_iface csp_skip_iface = {
        &csp_skip_initials, &csp_skip_afters, &csp_skip_get_id,
        &csp_skip_ud_size,  &csp_skip_init,   &csp_skip_done};

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process {
    const struct csp_process_iface iface;
};

#define csp_process_ud(proc) ((void *) (((struct csp_process *) (proc)) + 1))

static struct csp_process *
csp_process_new(struct csp *csp, const void *temp_ud,
                const struct csp_process_iface *iface)
{
    size_t ud_size = iface->ud_size(csp, temp_ud);
    struct csp_process *process = malloc(sizeof(struct csp_process) + ud_size);
    assert(process != NULL);
    iface->init_ud(csp, csp_process_ud(process), temp_ud);
    *((struct csp_process_iface *) &process->iface) = *iface;
    return process;
}

static void
csp_process_free(struct csp *csp, struct csp_process *process)
{
    process->iface.done_ud(csp, csp_process_ud(process));
    free(process);
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

static struct csp_process *
csp_process_get(struct csp_priv *csp, csp_id process_id);

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
    csp->public.stop =
            csp_process_init(&csp->public, NULL, NULL, &csp_stop_iface);
    csp->stop = csp_process_get(csp, csp->public.stop);
    csp->public.skip =
            csp_process_init(&csp->public, NULL, NULL, &csp_skip_iface);
    csp->skip = csp_process_get(csp, csp->public.skip);
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

csp_id
csp_process_init(struct csp *pcsp, const void *temp_ud, void **ud,
                 const struct csp_process_iface *iface)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process **process;
    csp_id process_id = iface->get_id(&csp->public, temp_ud);
    process = csp_id_process_map_at(&csp->processes, process_id);
    if (*process == NULL) {
        *process = csp_process_new(&csp->public, temp_ud, iface);
    }
    if (ud != NULL) {
        *ud = csp_process_ud(*process);
    }
    return process_id;
}

static struct csp_process *
csp_process_get(struct csp_priv *csp, csp_id process_id)
{
    struct csp_process *process =
            csp_id_process_map_get(&csp->processes, process_id);
    /* It's an error to do something with a process that you haven't created
     * yet. */
    assert(process != NULL);
    return process;
}

void
csp_process_build_initials(struct csp *pcsp, csp_id process_id,
                           struct csp_id_set *set)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process *process = csp_process_get(csp, process_id);
    process->iface.initials(&csp->public, set, csp_process_ud(process));
}

void
csp_process_build_afters(struct csp *pcsp, csp_id process_id, csp_id initial,
                         struct csp_id_set *set)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process *process = csp_process_get(csp, process_id);
    process->iface.afters(&csp->public, initial, set, csp_process_ud(process));
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
