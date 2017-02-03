/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <stdlib.h>

#include "ccan/likely/likely.h"
#include "Judy.h"

/* We're going to use a simple free list to reduce the number of times we dive
 * down in malloc, especially since we're going to build up and free sets quite
 * a lot during a refinement check.
 *
 * Judy allocates everything in units of Words, so we need a separate free list
 * for each distinct Word size.  The small ones seem to be the most common, so
 * we only keep free lists for the sizes up through a hopefully reasonable
 * number, and use calloc/free directly for everything bigger than that. */

#define MAX_WORDS 64
static void *FREE_LISTS[MAX_WORDS + 1];

static void *
new_object(size_t words)
{
    return calloc(words, sizeof(Word_t));
}

static void *
reuse_object(size_t words)
{
    void *object = FREE_LISTS[words];
    FREE_LISTS[words] = *((void **) object);
    return object;
}

Word_t
JudyMalloc(Word_t words)
{
    if (unlikely(words > MAX_WORDS || FREE_LISTS[words] == NULL)) {
        return (Word_t) new_object(words);
    } else {
        return (Word_t) reuse_object(words);
    }
}

void
JudyFree(void *object, Word_t words)
{
    if (unlikely(words > MAX_WORDS)) {
        free(object);
    } else {
        *((void **) object) = FREE_LISTS[words];
        FREE_LISTS[words] = object;
    }
}

Word_t
JudyMallocVirtual(Word_t words)
{
    return JudyMalloc(words);
}

void
JudyFreeVirtual(void *object, Word_t words)
{
    JudyFree(object, words);
}
