/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_H
#define HST_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/*------------------------------------------------------------------------------
 * Environments
 */

/* Each process and event is identified by a number. */
typedef unsigned long csp_id;

struct csp {
    csp_id  tau;
    csp_id  tick;
    csp_id  skip;
    csp_id  stop;
};

struct csp *
csp_new(void);

void
csp_free(struct csp *csp);

/* Return the ID of the event with the given name.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
csp_id
csp_get_event_id(struct csp *csp, const char *name);

/* Return the name of the event with the given ID.  If you haven't created an
 * event with that ID (via csp_get_event_id), we return NULL. */
const char *
csp_get_event_name(struct csp *csp, csp_id event);

/*------------------------------------------------------------------------------
 * Sets
 */

/* A set of IDs, stored as a sorted array.  This type is read-only; to construct
 * a set, use a csp_id_set_builder. */
struct csp_id_set {
    csp_id  *ids;
    size_t  count;
    size_t  allocated_count;
};

void
csp_id_set_init(struct csp_id_set *set);

void
csp_id_set_done(struct csp_id_set *set);

/* A writeable view of a set of IDs. */
struct csp_id_set_builder {
    void  *working_set;
};

/* Initialize and clear a set builder. */
void
csp_id_set_builder_init(struct csp_id_set_builder *builder);

void
csp_id_set_builder_done(struct csp_id_set_builder *builder);

/* Add a single ID to a set builder. */
void
csp_id_set_builder_add(struct csp_id_set_builder *builder, csp_id id);

/* Add several IDs to a set builder.  `ids` does not need to be sorted, and it's
 * okay for it to contain duplicates. */
void
csp_id_set_builder_add_many(struct csp_id_set_builder *builder, size_t count,
                            csp_id *ids);

/* Add the contents of an existing set to a set builder. */
void
csp_id_set_builder_merge(struct csp_id_set_builder *builder,
                         const struct csp_id_set *set);

/* "Lock" the contents of a set builder, filling in a (read-only)
 * set.  The set builder will be cleared in the process, allowing you to
 * reuse it if you need to build several sets. */
int
csp_id_set_build(struct csp_id_set *set, struct csp_id_set_builder *builder);

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp_process_iface {
    void
    (*initials)(struct csp *csp, struct csp_id_set_builder *builder, void *ud);

    void
    (*afters)(struct csp *csp, csp_id initial,
              struct csp_id_set_builder *builder, void *ud);

    void
    (*free_ud)(void *ud);
};

/* Returns a new reference to `process`.  You are responsible for obtaining a
 * unique ID for the process. */
void
csp_process_init(struct csp *csp, csp_id process, void *ud,
                 const struct csp_process_iface *iface);

/* Returns a new reference to `process`.  You retain your original reference to
 * `process`. */
csp_id
csp_process_ref(struct csp *csp, csp_id process);

/* Releases your reference to `process`. */
void
csp_process_deref(struct csp *csp, csp_id process);

/* Releases your references to all of `processes` in a set. */
void
csp_process_deref_set(struct csp *csp, struct csp_id_set *processes);

void
csp_process_get_initials(struct csp *csp, csp_id process,
                         struct csp_id_set *dest);

/* Returns a new reference to all of the processes in `dest`. */
void
csp_process_get_afters(struct csp *csp, csp_id process, csp_id initial,
                       struct csp_id_set *dest);

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

struct csp_id_scope { unsigned int unused; };

csp_id
csp_id_start(struct csp_id_scope *scope);

csp_id
csp_id_add_id(csp_id id, csp_id id_to_add);

csp_id
csp_id_add_id_set(csp_id id, const struct csp_id_set *set);

csp_id
csp_id_add_name(csp_id id, const char *name);

#ifdef __cplusplus
}
#endif
#endif /* HST_H */
