/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "operators/recursion.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/container_of/container_of.h"
#include "ccan/likely/likely.h"
#include "basics.h"
#include "hst.h"

/*------------------------------------------------------------------------------
 * Recursive process
 */

/* A "recursive process" is one that has been assigned a name inside of some
 * recursion scope.  The act of naming the process and providing its definition
 * are separate steps; this allows you to refer to the name while creating its
 * definition (i.e., recursion!).  In the operational semantics, this is kind of
 * like a forward declaration. */

struct csp_recursive_process {
    csp_id definition;
};

static void
csp_recursive_process_initials(struct csp *csp, struct csp_id_set *set,
                               void *vrecursive_process)
{
    struct csp_recursive_process *recursive_process = vrecursive_process;
    assert(recursive_process->definition != CSP_PROCESS_NONE);
    csp_process_build_initials(csp, recursive_process->definition, set);
}

static void
csp_recursive_process_afters(struct csp *csp, csp_id initial,
                             struct csp_id_set *set, void *vrecursive_process)
{
    struct csp_recursive_process *recursive_process = vrecursive_process;
    assert(recursive_process->definition != CSP_PROCESS_NONE);
    csp_process_build_afters(csp, recursive_process->definition, initial, set);
}

static csp_id
csp_recursive_process_get_id(struct csp *csp, const void *vinput)
{
    const csp_id *input = vinput;
    return *input;
}

static size_t
csp_recursive_process_ud_size(struct csp *csp, const void *vinput)
{
    return sizeof(struct csp_recursive_process);
}

static void
csp_recursive_process_init(struct csp *csp, void *vrecursive_process,
                           const void *vinput)
{
    struct csp_recursive_process *recursive_process = vrecursive_process;
    recursive_process->definition = CSP_PROCESS_NONE;
}

static void
csp_recursive_process_done(struct csp *csp, void *vrecursive_process)
{
    /* nothing to do */
}

static const struct csp_process_iface csp_recursion_iface = {
        &csp_recursive_process_initials, &csp_recursive_process_afters,
        &csp_recursive_process_get_id,   &csp_recursive_process_ud_size,
        &csp_recursive_process_init,     &csp_recursive_process_done};

static void
csp_recursive_process(struct csp *csp, csp_id id,
                      struct csp_recursive_process **recursive_process)
{
    csp_process_init(csp, &id, (void **) recursive_process,
                     &csp_recursion_iface);
}

/*------------------------------------------------------------------------------
 * ID→recursive process map
 */

static void
csp_recursive_processes_init(struct csp_recursive_processes *processes)
{
    csp_map_init(&processes->map);
}

static void
csp_recursive_processes_done(struct csp_recursive_processes *processes)
{
    csp_map_done(&processes->map, NULL, NULL);
}

struct csp_recursive_process **
csp_recursive_processes_at(struct csp_recursive_processes *processes, csp_id id)
{
    return (struct csp_recursive_process **) csp_map_at(&processes->map, id);
}

struct csp_recursive_process *
csp_recursive_processes_get(struct csp_recursive_processes *processes,
                            csp_id id)
{
    return csp_map_get(&processes->map, id);
}

/*------------------------------------------------------------------------------
 * Recursion scope
 */

/* Annoying but necessary: keep this in sync with the "real" definition of
 * struct csp_priv from environment.c. */
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
    csp_recursive_processes_init(&scope->processes);
}

void
csp_recursion_scope_done(struct csp_recursion_scope *scope)
{
    csp_recursive_processes_done(&scope->processes);
}

csp_id
csp_recursion_create_id(csp_id scope, const char *name, size_t name_length)
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
    struct csp_recursive_process **recursive_process;
    csp_id id = csp_recursion_create_id(scope->scope, name, name_length);
    recursive_process = csp_recursive_processes_at(&scope->processes, id);
    if (*recursive_process == NULL) {
        csp_recursive_process(csp, id, recursive_process);
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
    struct csp_recursive_process *recursive_process;
    csp_id id = csp_recursion_create_id(scope->scope, name, name_length);
    recursive_process = csp_recursive_processes_get(&scope->processes, id);
    if (unlikely(recursive_process == NULL)) {
        return false;
    } else {
        if (likely(recursive_process->definition == CSP_PROCESS_NONE)) {
            scope->unfilled_count--;
            recursive_process->definition = process;
            return true;
        } else {
            return false;
        }
    }
}
