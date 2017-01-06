/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ENVIRONMENT_H
#define HST_ENVIRONMENT_H

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#include "id-set.h"
#include "process.h"

/*------------------------------------------------------------------------------
 * Environments
 */

/* Each process and event is identified by a number. */
typedef uint64_t csp_id;
#define CSP_ID_FMT "0x%016" PRIx64

#define CSP_ID_NONE ((csp_id) 0)
#define CSP_PROCESS_NONE CSP_ID_NONE

struct csp {
    csp_id tau;
    csp_id tick;
    csp_id skip;
    csp_id stop;
};

struct csp *
csp_new(void);

void
csp_free(struct csp *csp);

/* Return the ID of the event with the given name.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
csp_id
csp_get_event_id(struct csp *csp, const char *name);

/* Return the ID of the event with the given name.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
csp_id
csp_get_sized_event_id(struct csp *csp, const char *name, size_t name_length);

/* Return the name of the event with the given ID.  If you haven't created an
 * event with that ID (via csp_get_event_id), we return NULL. */
const char *
csp_get_event_name(struct csp *csp, csp_id event);

/* Register a process.  There must not already be a process registered with the
 * same ID. */
void
csp_register_process(struct csp *csp, struct csp_process *process);

/* Return the process registered with a particular ID, or NULL if there isn't
 * one. */
struct csp_process *
csp_get_process(struct csp *csp, csp_id id);

/* Return the process registered with a particular ID, which is required to
 * exist. */
struct csp_process *
csp_require_process(struct csp *csp, csp_id id);

void
csp_build_process_initials(struct csp *csp, csp_id process_id,
                           struct csp_id_set *set);

void
csp_build_process_afters(struct csp *csp, csp_id process_id, csp_id initial,
                         struct csp_id_set *set);

/*------------------------------------------------------------------------------
 * Constructing process IDs
 */

/* We want to use reproducible IDs for our processes, which only depend on the
 * definition of the process.  That is, if you try to define two processes with
 * exactly the same definition, you should end up with the same ID for each one,
 * without having to coordinate with anyone.
 *
 * That means we need some way to record what the definition of a process is,
 * and a way to translate those definitions into an ID.  We don't need a
 * super-precise definition of the process — for instance, we don't need the
 * full AST of a CSPM term.  It's enough to have a list of all of the "inputs"
 * that are needed for each kind of operator, and some tag to distinguish one
 * operator from another.
 *
 * The "scope" below is the operator tag.  You just declare a scope somewhere in
 * the file that deals with a particular operator.  For instance, the file that
 * defines the prefix (→) operator would include:
 *
 *     static struct csp_id_scope  prefix;
 *
 * You don't need to fill the struct in, it just needs to exist.  That scope
 * then provides a unique basis to generate IDs for all prefix processes.  The
 * prefix operator (a → B) has two inputs: the event `a` and the process `B`.
 * Both of those are represented internally by IDs, and so once you have the
 * prefix scope, and the IDs for event `a` and process `B`, you can easily
 * (and reproducibly) generate the ID for any prefix process:
 *
 *     static csp_id
 *     prefix_id(csp_id a, csp_id B)
 *     {
 *         csp_id  id = csp_id_start(&prefix);
 *         id = csp_id_add_id(id, a);
 *         id = csp_id_add_id(id, B);
 *         return id;
 *     }
 */

struct csp_id_scope {
    unsigned int unused;
};

csp_id
csp_id_start(struct csp_id_scope *scope);

csp_id
csp_id_add_id(csp_id id, csp_id id_to_add);

csp_id
csp_id_add_id_set(csp_id id, const struct csp_id_set *set);

csp_id
csp_id_add_name(csp_id id, const char *name);

csp_id
csp_id_add_name_sized(csp_id id, const char *name, size_t name_length);

#endif /* HST_ENVIRONMENT_H */
