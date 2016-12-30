/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_OPERATORS_H
#define HST_OPERATORS_H

#include <stdlib.h>

#include "basics.h"
#include "environment.h"
#include "map.h"

/*------------------------------------------------------------------------------
 * Operators
 */

/* In all of the below:  `p` and `q` are processes.  `ps` is a set of processes.
 * `a` is an event. */

csp_id
csp_external_choice(struct csp *csp, csp_id p, csp_id q);

csp_id
csp_internal_choice(struct csp *csp, csp_id p, csp_id q);

csp_id
csp_prefix(struct csp *csp, csp_id a, csp_id p);

csp_id
csp_replicated_external_choice(struct csp *csp, const struct csp_id_set *ps);

csp_id
csp_replicated_internal_choice(struct csp *csp, const struct csp_id_set *ps);

csp_id
csp_sequential_composition(struct csp *csp, csp_id p, csp_id q);

/*------------------------------------------------------------------------------
 * Recursion
 */

/* A "recursion scope" is the main building block that you need to create
 * mutually recursive processes.  You can create one or more "recursion targets"
 * within the scope, each of which maps a name to a process.  But importantly,
 * you don't have to know in advance which process you're going to map each name
 * to.  That lets you define a name for a process, and then use that same name
 * in the definition of the process.  Presto, recursion! */

struct csp_recursive_processes {
    struct csp_map map;
};

struct csp_recursion_scope {
    csp_id scope;
    size_t unfilled_count;
    struct csp_recursive_processes processes;
};

csp_id
csp_recursion_create_id(csp_id scope, const char *name, size_t name_length);

/* Initialize a new recursion scope.  You are responsible for passing in a
 * different `scope_id` for each recursion scope that you create. */
void
csp_recursion_scope_init(struct csp *csp, struct csp_recursion_scope *scope);

void
csp_recursion_scope_done(struct csp_recursion_scope *scope);

/* Return the process ID of the recursion target with the given name, creating
 * it if necessary.  The recursion target will initially be empty; you must
 * "fill" it by calling csp_recursion_scope_fill before destroying the scope. */
csp_id
csp_recursion_scope_get(struct csp *csp, struct csp_recursion_scope *scope,
                        const char *name);

/* Same as csp_recursion_scope_add, but providing an explicit length for `name`.
 * `name` does not need to be NUL-terminated, but it cannot contain any NULs. */
csp_id
csp_recursion_scope_get_sized(struct csp *csp,
                              struct csp_recursion_scope *scope,
                              const char *name, size_t name_length);

/* Fill a recursion target.  You must call this exactly once for each recursion
 * target that you create.  Returns false if this recursion target doesn't
 * exist, or if it has been filled already.  After this function returns, the
 * recursion target will behave exactly like `process`. */
bool
csp_recursion_scope_fill(struct csp_recursion_scope *scope, const char *name,
                         csp_id process);

/* Same as csp_recursion_scope_fill, but providing an explicit length for
 * `name`.  `name` does not need to be NUL-terminated, but it cannot contain any
 * NULs. */
bool
csp_recursion_scope_fill_sized(struct csp_recursion_scope *scope,
                               const char *name, size_t name_length,
                               csp_id process);

#endif /* HST_OPERATORS_H */
