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

struct csp;

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

/* Return the ID of the predefined τ ("tau") event. */
csp_id
csp_tau(struct csp *csp);

/* Return the ID of the predefined ✔ (tick) event. */
csp_id
csp_tick(struct csp *csp);

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
 * Operators and predefined processes
 */

/* Return a new reference to the predefined STOP process. */
csp_id
csp_stop(struct csp *csp);

/* Return a new reference to the predefined SKIP process. */
csp_id
csp_skip(struct csp *csp);

#ifdef __cplusplus
}
#endif
#endif /* HST_H */
