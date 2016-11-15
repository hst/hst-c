/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"
#include "ccan/container_of/container_of.h"
#include "ccan/hash/hash.h"
#include "ccan/likely/likely.h"
#include "hst.h"

struct csp_priv {
    struct csp public;
    csp_id next_recursion_scope_id;
};

void
csp_recursion_scope_init(struct csp *pcsp, struct csp_recursion_scope *scope)
{
    struct csp_priv *csp = container_of(pcsp, struct csp_priv, public);
    scope->scope = csp->next_recursion_scope_id++;
    scope->unfilled_count = 0;
    scope->names = NULL;
}

void
csp_recursion_scope_done(struct csp_recursion_scope *scope)
{
    UNNEEDED Word_t dummy;
    JLFA(dummy, scope->names);
}

struct csp_recursion {
    csp_id process;
};

static void
csp_recursion_initials(struct csp *csp, struct csp_id_set_builder *builder,
                       void *vrecursion)
{
    struct csp_recursion *recursion = vrecursion;
    assert(recursion->process != CSP_PROCESS_NONE);
    csp_process_build_initials(csp, recursion->process, builder);
}

static void
csp_recursion_afters(struct csp *csp, csp_id initial,
                     struct csp_id_set_builder *builder, void *vrecursion)
{
    struct csp_recursion *recursion = vrecursion;
    assert(recursion->process != CSP_PROCESS_NONE);
    csp_process_build_afters(csp, recursion->process, initial, builder);
}

static csp_id
csp_recursion_get_id(struct csp *csp, const void *vinput)
{
    const csp_id *input = vinput;
    return *input;
}

static size_t
csp_recursion_ud_size(struct csp *csp, const void *vinput)
{
    return sizeof(struct csp_recursion);
}

static void
csp_recursion_init(struct csp *csp, void *vrecursion, const void *vinput)
{
    struct csp_recursion *recursion = vrecursion;
    recursion->process = CSP_PROCESS_NONE;
}

static void
csp_recursion_done(struct csp *csp, void *vrecursion)
{
    /* nothing to do */
}

static const struct csp_process_iface csp_recursion_iface = {
        &csp_recursion_initials, &csp_recursion_afters, &csp_recursion_get_id,
        &csp_recursion_ud_size,  &csp_recursion_init,   &csp_recursion_done};

static void
csp_recursion(struct csp *csp, csp_id id, struct csp_recursion **recursion)
{
    csp_process_init(csp, &id, (void **) recursion, &csp_recursion_iface);
}

/* The double-underscore will cause the linker to not expose this as a public
 * part of the API, but we can still use it in other files.  We use this is the
 * CSP₀ to support the (somewhat-hacky, testing-only) X@0 syntax. */
csp_id
csp__recursion_create_id(csp_id scope, const char *name, size_t name_length)
{
    static struct csp_id_scope recursion;
    csp_id id = csp_id_start(&recursion);
    id = csp_id_add_id(id, scope);
    id = csp_id_add_name_sized(id, name, name_length);
    return id;
}

csp_id
csp_recursion_scope_get(struct csp *csp, struct csp_recursion_scope *scope,
                        const char *name)
{
    return csp_recursion_scope_get_sized(csp, scope, name, strlen(name));
}

csp_id
csp_recursion_scope_get_sized(struct csp *csp,
                              struct csp_recursion_scope *scope,
                              const char *name, size_t name_length)
{
    Word_t *vrecursion;
    csp_id id = csp__recursion_create_id(scope->scope, name, name_length);
    JLI(vrecursion, scope->names, id);
    if (*vrecursion == 0) {
        struct csp_recursion *recursion = NULL;
        csp_recursion(csp, id, &recursion);
        *vrecursion = (Word_t) recursion;
        scope->unfilled_count++;
    }
    return id;
}

bool
csp_recursion_scope_fill(struct csp_recursion_scope *scope, const char *name,
                         csp_id process)
{
    return csp_recursion_scope_fill_sized(scope, name, strlen(name), process);
}

bool
csp_recursion_scope_fill_sized(struct csp_recursion_scope *scope,
                               const char *name, size_t name_length,
                               csp_id process)
{
    Word_t *vrecursion;
    csp_id id = csp__recursion_create_id(scope->scope, name, name_length);
    JLG(vrecursion, scope->names, id);
    if (unlikely(vrecursion == NULL)) {
        return false;
    } else {
        struct csp_recursion *recursion = (void *) *vrecursion;
        if (likely(recursion->process == CSP_PROCESS_NONE)) {
            scope->unfilled_count--;
            recursion->process = process;
            return true;
        } else {
            return false;
        }
    }
}
