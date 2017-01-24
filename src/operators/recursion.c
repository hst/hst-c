/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "operators.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/container_of/container_of.h"
#include "ccan/likely/likely.h"
#include "basics.h"
#include "environment.h"
#include "event.h"
#include "macros.h"
#include "process.h"

/*------------------------------------------------------------------------------
 * Recursive process
 */

/* A "recursive process" is one that has been assigned a name inside of some
 * recursion scope.  The act of naming the process and providing its definition
 * are separate steps; this allows you to refer to the name while creating its
 * definition (i.e., recursion!).  In the operational semantics, this is kind of
 * like a forward declaration. */

struct csp_recursive_process {
    struct csp_process process;
    const char *name;
    struct csp_process *definition;
};

static void
csp_recursive_process_name(struct csp *csp, struct csp_process *process,
                           struct csp_name_visitor *visitor);

static void
csp_recursive_process_initials(struct csp *csp, struct csp_process *process,
                               struct csp_event_visitor *visitor)
{
    struct csp_recursive_process *recursive_process =
            container_of(process, struct csp_recursive_process, process);
    assert(recursive_process->definition != NULL);
    csp_process_visit_initials(csp, recursive_process->definition, visitor);
}

static void
csp_recursive_process_afters(struct csp *csp, struct csp_process *process,
                             const struct csp_event *initial,
                             struct csp_edge_visitor *visitor)
{
    struct csp_recursive_process *recursive_process =
            container_of(process, struct csp_recursive_process, process);
    assert(recursive_process->definition != NULL);
    csp_process_visit_afters(csp, recursive_process->definition, initial,
                             visitor);
}

static void
csp_recursive_process_free(struct csp *csp, struct csp_process *process)
{
    struct csp_recursive_process *recursive_process =
            container_of(process, struct csp_recursive_process, process);
    free((char *) recursive_process->name);
    free(recursive_process);
}

static const struct csp_process_iface csp_recursive_process_iface = {
        0, csp_recursive_process_name, csp_recursive_process_initials,
        csp_recursive_process_afters, csp_recursive_process_free};

static struct csp_process *
csp_recursive_process_new(struct csp *csp, const char *name, size_t name_length,
                          csp_id id)
{
    struct csp_recursive_process *recursive_process;
    char *name_copy;
    return_if_nonnull(csp_get_process(csp, id));
    recursive_process = malloc(sizeof(struct csp_recursive_process));
    assert(recursive_process != NULL);
    recursive_process->process.id = id;
    recursive_process->process.iface = &csp_recursive_process_iface;
    name_copy = malloc(name_length + 1);
    assert(name_copy != NULL);
    memcpy(name_copy, name, name_length);
    name_copy[name_length] = '\0';
    recursive_process->name = name_copy;
    recursive_process->definition = NULL;
    csp_register_process(csp, &recursive_process->process);
    return &recursive_process->process;
}

static void
csp_recursive_process(struct csp *csp, const char *name, size_t name_length,
                      csp_id id,
                      struct csp_recursive_process **recursive_process)
{
    struct csp_process *process =
            csp_recursive_process_new(csp, name, name_length, id);
    *recursive_process =
            container_of(process, struct csp_recursive_process, process);
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

static struct csp_recursive_process *
csp_recursive_processes_get(struct csp_recursive_processes *processes,
                            csp_id id)
{
    return csp_map_get(&processes->map, id);
}

static void
csp_recursive_processes_insert(struct csp_recursive_processes *processes,
                               csp_id id,
                               struct csp_recursive_process *recursive_process)
{
    csp_map_insert(&processes->map, id, recursive_process);
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

struct csp_process *
csp_recursion_scope_get(struct csp *csp, struct csp_recursion_scope *scope,
                        const char *name)
{
    return csp_recursion_scope_get_sized(csp, scope, name, strlen(name));
}

struct csp_process *
csp_recursion_scope_get_sized(struct csp *csp,
                              struct csp_recursion_scope *scope,
                              const char *name, size_t name_length)
{
    struct csp_recursive_process *recursive_process;
    csp_id id = csp_recursion_create_id(scope->scope, name, name_length);
    recursive_process = csp_recursive_processes_get(&scope->processes, id);
    if (recursive_process == NULL) {
        csp_recursive_process(csp, name, name_length, id, &recursive_process);
        csp_recursive_processes_insert(&scope->processes, id,
                                       recursive_process);
        scope->unfilled_count++;
    }
    return &recursive_process->process;
}

bool
csp_recursion_scope_fill(struct csp_recursion_scope *scope, const char *name,
                         struct csp_process *process)
{
    return csp_recursion_scope_fill_sized(scope, name, strlen(name), process);
}

bool
csp_recursion_scope_fill_sized(struct csp_recursion_scope *scope,
                               const char *name, size_t name_length,
                               struct csp_process *process)
{
    struct csp_recursive_process *recursive_process;
    csp_id id = csp_recursion_create_id(scope->scope, name, name_length);
    recursive_process = csp_recursive_processes_get(&scope->processes, id);
    if (unlikely(recursive_process == NULL)) {
        return false;
    } else {
        if (likely(recursive_process->definition == NULL)) {
            scope->unfilled_count--;
            recursive_process->definition = process;
            return true;
        } else {
            return false;
        }
    }
}

/*------------------------------------------------------------------------------
 * Recursive process name
 */

struct csp_find_recursive_processes {
    struct csp_process_visitor visitor;
    struct csp_process_set *set;
};

static void
csp_find_recursive_processes_visit(struct csp *csp,
                                   struct csp_process_visitor *visitor,
                                   struct csp_process *process)
{
    struct csp_find_recursive_processes *self =
            container_of(visitor, struct csp_find_recursive_processes, visitor);
    if (process->iface == &csp_recursive_process_iface) {
        csp_process_set_add(self->set, process);
    }
}

static struct csp_find_recursive_processes
csp_find_recursive_processes(struct csp_process_set *set)
{
    struct csp_find_recursive_processes self = {
            {csp_find_recursive_processes_visit}, set};
    return self;
}

struct csp_recursive_name {
    struct csp_name_visitor visitor;
    struct csp_name_visitor *wrapped;
};

static void
csp_recursive_name_visit(struct csp *csp, struct csp_name_visitor *visitor,
                         const char *str, size_t length)
{
    struct csp_recursive_name *self =
            container_of(visitor, struct csp_recursive_name, visitor);
    csp_name_visitor_call_sized(csp, self->wrapped, str, length);
}

static struct csp_recursive_name
csp_recursive_name(struct csp_name_visitor *wrapped)
{
    struct csp_recursive_name self = {{csp_recursive_name_visit}, wrapped};
    return self;
}

static void
csp_recursive_process_name(struct csp *csp, struct csp_process *process,
                           struct csp_name_visitor *visitor)
{
    struct csp_recursive_process *recursive_process =
            container_of(process, struct csp_recursive_process, process);
    struct csp_process_set recursive_processes;
    struct csp_find_recursive_processes find;
    struct csp_recursive_name recurse;
    size_t count;
    struct csp_process **sorted;
    size_t i;

    /* If we're in the middle of visiting using a `csp_recursive_name` helper,
     * then we just need to output the name of the process.  (That's what the
     * recursive reference will look like in the middle of the process's
     * definition.) */
    if (visitor->visit == csp_recursive_name_visit) {
        csp_name_visitor_call(csp, visitor, recursive_process->name);
        return;
    }

    /* Otherwise we need to output the `let` statement that contains all of the
     * definitions that are mutually recursive with the current one.  We'll
     * first do a quick BFS to find them all. */
    csp_process_set_init(&recursive_processes);
    find = csp_find_recursive_processes(&recursive_processes);
    csp_process_bfs(csp, process, &find.visitor);
    csp_process_set_sort_by_index(&recursive_processes, &count, &sorted);
    csp_process_set_done(&recursive_processes);

    recurse = csp_recursive_name(visitor);
    csp_name_visitor_call(csp, &recurse.visitor, "let");
    for (i = 0; i < count; i++) {
        struct csp_recursive_process *current_process =
                container_of(sorted[i], struct csp_recursive_process, process);
        csp_name_visitor_call(csp, &recurse.visitor, " ");
        csp_name_visitor_call(csp, &recurse.visitor, current_process->name);
        csp_name_visitor_call(csp, &recurse.visitor, "=");
        csp_process_name(csp, current_process->definition, &recurse.visitor);
    }
    csp_name_visitor_call(csp, &recurse.visitor, " within ");
    csp_name_visitor_call(csp, &recurse.visitor, recursive_process->name);
    free(sorted);
}
