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
 * Sets
 */

/* Each process and event is identified by a number. */
typedef unsigned long csp_id;

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

#ifdef __cplusplus
}
#endif
#endif /* HST_H */
