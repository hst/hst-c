/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_ID_PAIR_H
#define HST_ID_PAIR_H

#include <stdbool.h>
#include <stdlib.h>

#include "basics.h"

/*------------------------------------------------------------------------------
 * Pairs
 */

struct csp_id_pair {
    csp_id from;
    csp_id to;
};

/* We preallocate a certain number of entries in the csp_id_pair_array struct
 * itself, to minimize malloc overhead for small arrays.  We automatically
 * calculate a number that makes the size of the csp_id_pair_array itself a nice
 * multiple of sizeof(void *).  On 64-bit platforms this should currently
 * evaluate to 6. */
#define CSP_ID_PAIR_ARRAY_INTERNAL_SIZE                        \
    (((sizeof(void *) * 16)          /* target overall size */ \
      - sizeof(struct csp_id_pair *) /* pairs */               \
      - sizeof(size_t)               /* count */               \
      - sizeof(size_t)               /* allocated_count */     \
      ) /                                                      \
     sizeof(struct csp_id_pair))

/* An array of pairs of IDs. */
struct csp_id_pair_array {
    struct csp_id_pair *pairs;
    size_t count;
    size_t allocated_count;
    struct csp_id_pair internal[CSP_ID_PAIR_ARRAY_INTERNAL_SIZE];
};

void
csp_id_pair_array_init(struct csp_id_pair_array *array);

void
csp_id_pair_array_done(struct csp_id_pair_array *array);

/* Ensure that the array's `pairs` field has enough allocated space to hold
 * `count` pairs.  The array's `count` will be set to `count` after this
 * returns; you must then fill in the `pairs` field with the actual ID pairs. */
void
csp_id_pair_array_ensure_size(struct csp_id_pair_array *array, size_t count);

bool
csp_id_pair_array_eq(const struct csp_id_pair_array *a1,
                     const struct csp_id_pair_array *a2);

/* We preallocate a certain number of entries in the csp_id_pair_set struct
 * itself, to minimize malloc overhead for small sets.  We automatically
 * calculate a number that makes the size of the csp_id_pair_set itself a nice
 * multiple of sizeof(void *).  On 64-bit platforms this should currently
 * evaluate to 6. */
#define CSP_ID_PAIR_SET_INTERNAL_SIZE                        \
    (((sizeof(void *) * 16)          /* target overall size */ \
      - sizeof(struct csp_id_pair *) /* pairs */               \
      - sizeof(size_t)               /* count */               \
      - sizeof(size_t)               /* allocated_count */     \
      ) /                                                      \
     sizeof(struct csp_id_pair))

/* An set of pairs of IDs. */
struct csp_id_pair_set {
    struct csp_id_pair *pairs;
    size_t count;
    size_t allocated_count;
    struct csp_id_pair internal[CSP_ID_PAIR_SET_INTERNAL_SIZE];
};

void
csp_id_pair_set_init(struct csp_id_pair_set *set);

void
csp_id_pair_set_done(struct csp_id_pair_set *set);

/* Update `dest` to contain the union of `dest` and `src`. */
void
csp_id_pair_set_union(struct csp_id_pair_set *dest,
                      const struct csp_id_pair_set *src);

bool
csp_id_pair_set_contains(const struct csp_id_pair_set *set,
                         struct csp_id_pair pair);

bool
csp_id_pair_set_eq(const struct csp_id_pair_set *set1,
                   const struct csp_id_pair_set *set2);

/* A writeable view of a set of pairs of IDs. */
struct csp_id_pair_set_builder {
    size_t count;
    void *working_set;
};

/* Initialize and clear a set builder. */
void
csp_id_pair_set_builder_init(struct csp_id_pair_set_builder *builder);

void
csp_id_pair_set_builder_done(struct csp_id_pair_set_builder *builder);

/* Add a single pair to a set builder. */
void
csp_id_pair_set_builder_add(struct csp_id_pair_set_builder *builder,
                            struct csp_id_pair pair);

/* "Lock" the contents of a pair set builder, filling in a (read-only)
 * set.  The set builder will be cleared in the process, allowing you to
 * reuse it if you need to build several sets. */
void
csp_id_pair_set_build(struct csp_id_pair_set *set,
                      struct csp_id_pair_set_builder *builder);

#endif /* HST_ID_PAIR_H */
