/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"
#include "ccan/container_of/container_of.h"
#include "ccan/hash/hash.h"
#include "ccan/likely/likely.h"
#include "hst.h"

static uint64_t
hash_name(const char* name)
{
    return hash64_any(name, strlen(name), 0);
}

struct csp_process {
    void  *ud;
    const struct csp_process_iface  iface;
    unsigned int  ref_count;
};

static struct csp_process *
csp_process_new(void *ud, const struct csp_process_iface *iface)
{
    struct csp_process  *process = malloc(sizeof(struct csp_process));
    assert(process != NULL);
    process->ud = ud;
    *((struct csp_process_iface *) &process->iface) = *iface;
    process->ref_count = 1;
    return process;
}

static void
csp_process_free(struct csp_process *process)
{
    if (process->iface.free_ud != NULL) {
        process->iface.free_ud(process->ud);
    }
    free(process);
}

struct csp_priv {
    struct csp  public;
    struct csp_id_set_builder  builder;
    void  *events;
    void  *processes;
    struct csp_process  *stop;
    struct csp_process  *skip;
};

static struct csp_process *
csp_process_get(struct csp_priv *csp, csp_id process_id);

static void
csp_stop_initials(struct csp *pcsp, struct csp_id_set_builder *builder,
                  void *ud)
{
}

static void
csp_stop_afters(struct csp *pcsp, csp_id initial,
                struct csp_id_set_builder *builder, void *ud)
{
}

const struct csp_process_iface csp_stop_iface = {
    &csp_stop_initials,
    &csp_stop_afters,
    NULL
};

static void
csp_skip_initials(struct csp *pcsp, struct csp_id_set_builder *builder,
                  void *ud)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    csp_id_set_builder_add(builder, csp->public.tick);
}

static void
csp_skip_afters(struct csp *pcsp, csp_id initial,
                struct csp_id_set_builder *builder, void *ud)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    if (initial == csp->public.tick) {
        csp_id_set_builder_add(builder, csp->public.stop);
    }
}

const struct csp_process_iface csp_skip_iface = {
    &csp_skip_initials,
    &csp_skip_afters,
    NULL
};

struct csp *
csp_new(void)
{
    static const char* const TAU = "τ";
    static const char* const TICK = "✔";
    struct csp_priv  *csp = malloc(sizeof(struct csp_priv));
    if (unlikely(csp == NULL)) {
        return NULL;
    }
    csp_id_set_builder_init(&csp->builder);
    csp->events = NULL;
    csp->processes = NULL;
    csp->public.tau = csp_get_event_id(&csp->public, TAU);
    csp->public.tick = csp_get_event_id(&csp->public, TICK);
    csp->public.stop = hash_name("STOP");
    csp_process_init(&csp->public, csp->public.stop, NULL, &csp_stop_iface);
    csp->stop = csp_process_get(csp, csp->public.stop);
    csp->public.skip = hash_name("SKIP");
    csp_process_init(&csp->public, csp->public.skip, NULL, &csp_skip_iface);
    csp->skip = csp_process_get(csp, csp->public.skip);
    return &csp->public;
}

void
csp_free(struct csp *pcsp)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    UNNEEDED Word_t  dummy;

    {
        Word_t  *vname;
        csp_id  event = 0;
        JLF(vname, csp->events, event);
        while (vname != NULL) {
            char  *name = (void *) *vname;
            free(name);
            JLN(vname, csp->events, event);
        }
        JLFA(dummy, csp->events);
    }

    /* Release the environment's references to the predefined STOP and SKIP
     * processes. */
    csp_process_deref(&csp->public, csp->public.stop);
    csp_process_deref(&csp->public, csp->public.skip);

    /* All of the other processes should have already been deferenced. */
    assert(csp->processes == NULL);

    csp_id_set_builder_done(&csp->builder);
    free(csp);
}

csp_id
csp_get_event_id(struct csp *pcsp, const char *name)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    csp_id  event = hash_name(name);
    Word_t  *vname;
    JLI(vname, csp->events, event);
    if (*vname == 0) {
        const char  *name_copy = strdup(name);
        *vname = (Word_t) name_copy;
    }
    return event;
}

const char *
csp_get_event_name(struct csp *pcsp, csp_id event)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    Word_t  *vname;
    JLG(vname, csp->events, event);
    if (vname == NULL) {
        return NULL;
    } else {
        return (void *) *vname;
    }
}

void
csp_process_init(struct csp *pcsp, csp_id process_id, void *ud,
                 const struct csp_process_iface *iface)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    Word_t  *vprocess;
    JLI(vprocess, csp->processes, process_id);
    if (*vprocess == 0) {
        struct csp_process  *process = csp_process_new(ud, iface);
        *vprocess = (Word_t) process;
    } else {
        struct csp_process  *process = (void *) *vprocess;
        process->ref_count++;
    }
}

static struct csp_process *
csp_process_get(struct csp_priv *csp, csp_id process_id)
{
    Word_t  *vprocess;
    JLG(vprocess, csp->processes, process_id);
    /* You must hold a reference to process_id, and every process with a
     * non-zero reference count must be in the Judy array. */
    assert(vprocess != NULL);
    return (void *) *vprocess;
}

csp_id
csp_process_ref(struct csp *pcsp, csp_id process_id)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process  *process = csp_process_get(csp, process_id);
    process->ref_count++;
    return process_id;
}

static void
csp_process_deref_one(struct csp_priv *csp, csp_id process_id,
                      struct csp_process *process)
{
    assert(process->ref_count > 0);
    if (--process->ref_count == 0) {
        UNNEEDED int  rc;
        csp_process_free(process);
        JLD(rc, csp->processes, process_id);
    }
}

void
csp_process_deref(struct csp *pcsp, csp_id process_id)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process  *process = csp_process_get(csp, process_id);
    csp_process_deref_one(csp, process_id, process);
}

void
csp_process_deref_set(struct csp *pcsp, struct csp_id_set *process_ids)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    size_t  i;
    for (i = 0; i < process_ids->count; i++) {
        csp_id  process_id = process_ids->ids[i];
        struct csp_process  *process = csp_process_get(csp, process_id);
        csp_process_deref_one(csp, process_id, process);
    }
}

void
csp_process_get_initials(struct csp *pcsp, csp_id process_id,
                         struct csp_id_set *dest)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process  *process = csp_process_get(csp, process_id);
    process->iface.initials(&csp->public, &csp->builder, process->ud);
    csp_id_set_build(dest, &csp->builder);
}

void
csp_process_get_afters(struct csp *pcsp, csp_id process_id, csp_id initial,
                       struct csp_id_set *dest)
{
    struct csp_priv  *csp = container_of(pcsp, struct csp_priv, public);
    struct csp_process  *process = csp_process_get(csp, process_id);
    process->iface.afters(&csp->public, initial, &csp->builder, process->ud);
    csp_id_set_build(dest, &csp->builder);
}
