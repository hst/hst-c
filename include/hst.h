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

#define CSP_PROCESS_NONE  ((csp_id) 0)

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

/* Return the ID of the event with the given name.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
csp_id
csp_get_sized_event_id(struct csp *csp, const char *name, size_t name_length);

/* Return the name of the event with the given ID.  If you haven't created an
 * event with that ID (via csp_get_event_id), we return NULL. */
const char *
csp_get_event_name(struct csp *csp, csp_id event);

/* Add a name for an existing process.  Not every process will have a name, and
 * each process might have more than one name.  Returns false if there is
 * already a process with the given name. */
bool
csp_add_process_name(struct csp *csp, csp_id process, const char *name);

/* Add a name for an existing process.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  Not every process will have
 * a name, and each process might have more than one name.  Returns false if
 * there is already a process with the given name. */
bool
csp_add_process_sized_name(struct csp *csp, csp_id process, const char *name,
                           size_t name_length);

/* Return the ID of the process with the given name.  Returns `CSP_PROCESS_NONE`
 * if there is no process with that name. */
csp_id
csp_get_process_by_name(struct csp *csp, const char *name);

/* Return the ID of the process with the given name.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  Returns `CSP_PROCESS_NONE`
 * if there is no process with that name. */
csp_id
csp_get_process_by_sized_name(struct csp *csp, const char *name,
                              size_t name_length);

/*------------------------------------------------------------------------------
 * Sets
 */

/* We preallocate a certain number of entries in the csp_id_set struct itself,
 * to minimize malloc overhead for small sets.  We've chosen 13 to make the set
 * of the csp_id_set itself a nice multiple of sizeof(void*). */
#define CSP_ID_SET_INTERNAL_SIZE  13

/* A set of IDs, stored as a sorted array.  This type is read-only; to construct
 * a set, use a csp_id_set_builder. */
struct csp_id_set {
    csp_id  *ids;
    size_t  count;
    size_t  allocated_count;
    csp_id  internal[CSP_ID_SET_INTERNAL_SIZE];
};

void
csp_id_set_init(struct csp_id_set *set);

void
csp_id_set_done(struct csp_id_set *set);

bool
csp_id_set_eq(const struct csp_id_set *set1, const struct csp_id_set *set2);

/* Fills a set with a copy of another set, without having to go through a
 * builder first.  You must have already initialized `set`.  This is guaranteed
 * to be equivalent to (and likely more efficient than):
 *
 *     struct csp_id_set_builder  builder;
 *     csp_id_set_builder_init(&builder);
 *     csp_id_set_builder_merge(&builder, other);
 *     csp_id_set_build(set, &builder);
 *     csp_id_set_builder_done(&builder);
 */
void
csp_id_set_clone(struct csp_id_set *set, const struct csp_id_set *other);

/* Shortcut for constructing an ID set with one element, without having to go
 * through a builder.  This is guaranteed to be equivalent to (and likely more
 * efficient than):
 *
 *     struct csp_id_set_builder  builder;
 *     csp_id_set_builder_init(&builder);
 *     csp_id_set_builder_add(&builder, event);
 *     csp_id_set_build(set, &builder);
 *     csp_id_set_builder_done(&builder);
 */
void
csp_id_set_fill_single(struct csp_id_set *set, csp_id event);

/* Shortcut for constructing an ID set with two elements, without having to go
 * through a builder.  This is guaranteed to be equivalent to (and likely more
 * efficient than):
 *
 *     struct csp_id_set_builder  builder;
 *     csp_id_set_builder_init(&builder);
 *     csp_id_set_builder_add(&builder, e1);
 *     csp_id_set_builder_add(&builder, e2);
 *     csp_id_set_build(set, &builder);
 *     csp_id_set_builder_done(&builder);
 */
void
csp_id_set_fill_double(struct csp_id_set *set, csp_id e1, csp_id e2);

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

/* Remove a single ID from a set builder.  Returns whether that ID was in the
 * builder or not. */
bool
csp_id_set_builder_remove(struct csp_id_set_builder *builder, csp_id id);

/* Remove several IDs from a set builder.  `ids` does not need to be sorted, and
 * it's okay for it to contain duplicates. */
void
csp_id_set_builder_remove_many(struct csp_id_set_builder *builder, size_t count,
                               csp_id *ids);

/* Add the contents of an existing set to a set builder. */
void
csp_id_set_builder_merge(struct csp_id_set_builder *builder,
                         const struct csp_id_set *set);

/* "Lock" the contents of a set builder, filling in a (read-only)
 * set.  The set builder will be cleared in the process, allowing you to
 * reuse it if you need to build several sets. */
void
csp_id_set_build(struct csp_id_set *set, struct csp_id_set_builder *builder);

/* "Lock" the contents of a set builder, filling in a (read-only)
 * set.  The set builder will NOT be cleared in the process, allowing you to
 * easily build several sets that have similar contents. */
void
csp_id_set_build_and_keep(struct csp_id_set *set,
                          struct csp_id_set_builder *builder);

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
    (*free_ud)(struct csp *csp, void *ud);
};

/* Creates a new process.  You are responsible for obtaining a unique ID for the
 * process.  If there is already an existing process with the same ID, it takes
 * precedence, and your new process is ignored and freed. */
void
csp_process_init(struct csp *csp, csp_id process, void *ud,
                 const struct csp_process_iface *iface);

void
csp_process_build_initials(struct csp *csp, csp_id process,
                           struct csp_id_set_builder *builder);

void
csp_process_build_afters(struct csp *csp, csp_id process, csp_id initial,
                         struct csp_id_set_builder *builder);

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
 * CSP₀
 */

/* Load in a CSP₀ process from an in-memory string, placing the ID of the new
 * process in `dest`.  Returns 0 on success.  If the CSP₀ process is invalid,
 * returns -1. */
int
csp_load_csp0_string(struct csp *csp, const char *str, csp_id *dest);

#ifdef __cplusplus
}
#endif
#endif /* HST_H */
