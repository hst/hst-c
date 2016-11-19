/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#include "ccan/likely/likely.h"
#include "hst.h"

void
csp_id_pair_array_init(struct csp_id_pair_array *array)
{
    array->pairs = array->internal;
    array->allocated_count = CSP_ID_PAIR_ARRAY_INTERNAL_SIZE;
    array->count = 0;
}

void
csp_id_pair_array_done(struct csp_id_pair_array *array)
{
    if (array->pairs != array->internal) {
        free(array->pairs);
    }
}

#define CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT 32

void
csp_id_pair_array_ensure_size(struct csp_id_pair_array *array, size_t count)
{
    array->count = count;
    if (unlikely(count > array->allocated_count)) {
        if (array->pairs == array->internal) {
            size_t new_count = CSP_ID_PAIR_ARRAY_FIRST_ALLOCATION_COUNT;
            while (count > new_count) {
                new_count *= 2;
            }
            array->pairs = malloc(new_count * sizeof(struct csp_id_pair));
            assert(array->pairs != NULL);
            array->allocated_count = new_count;
        } else {
            /* Whenever we reallocate, at least double the size of the existing
             * array. */
            struct csp_id_pair *new_pairs;
            size_t new_count = array->allocated_count;
            do {
                new_count *= 2;
            } while (count > new_count);
            new_pairs = realloc(array->pairs,
                                new_count * sizeof(struct csp_id_pair));
            assert(new_pairs != NULL);
            array->pairs = new_pairs;
            array->allocated_count = new_count;
        }
    }
}

bool
csp_id_pair_array_eq(const struct csp_id_pair_array *a1,
                     const struct csp_id_pair_array *a2)
{
    size_t i;
    if (a1->count != a2->count) {
        return false;
    }
    for (i = 0; i < a1->count; i++) {
        if (a1->pairs[i].from != a2->pairs[i].from ||
            a1->pairs[i].to != a2->pairs[i].to) {
            return false;
        }
    }
    return true;
}
