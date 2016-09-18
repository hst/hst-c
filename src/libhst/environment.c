/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"
#include "ccan/hash/hash.h"
#include "ccan/likely/likely.h"
#include "hst.h"

static uint64_t
hash_name(const char* name)
{
    return hash64_any(name, strlen(name), 0);
}

struct csp {
    void  *events;
    csp_id  tau;
    csp_id  tick;
};

struct csp *
csp_new(void)
{
    static const char* const TAU = "τ";
    static const char* const TICK = "✔";
    struct csp  *csp = malloc(sizeof(struct csp));
    if (unlikely(csp == NULL)) {
        return NULL;
    }
    csp->events = NULL;
    csp->tau = csp_get_event_id(csp, TAU);
    csp->tick = csp_get_event_id(csp, TICK);
    return csp;
}

void
csp_free(struct csp *csp)
{
    UNNEEDED Word_t  dummy;
    Word_t  *vname;
    csp_id  event = 0;
    JLF(vname, csp->events, event);
    while (vname != NULL) {
        char  *name = (void *) *vname;
        free(name);
        JLN(vname, csp->events, event);
    }
    JLFA(dummy, csp->events);
    free(csp);
}

csp_id
csp_get_event_id(struct csp *csp, const char *name)
{
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
csp_get_event_name(struct csp *csp, csp_id event)
{
    Word_t  *vname;
    JLG(vname, csp->events, event);
    if (vname == NULL) {
        return NULL;
    } else {
        return (void *) *vname;
    }
}

csp_id
csp_tau(struct csp *csp)
{
    return csp->tau;
}

csp_id
csp_tick(struct csp *csp)
{
    return csp->tick;
}
