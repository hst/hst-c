/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "equivalence.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/likely/likely.h"
#include "id-set.h"
#include "map.h"
#include "process.h"

static void
csp_process_classes_init(struct csp_process_classes *classes)
{
    csp_map_init(&classes->map);
}

static void
csp_process_classes_done(struct csp_process_classes *classes)
{
    csp_map_done(&classes->map, NULL, NULL);
}

static csp_id
csp_process_classes_get(const struct csp_process_classes *classes,
                        struct csp_process *process)
{
    void *entry = csp_map_get(&classes->map, (uintptr_t) process);
    if (entry == NULL) {
        return CSP_ID_NONE;
    } else {
        return (uintptr_t) entry;
    }
}

static csp_id
csp_process_classes_insert(struct csp_process_classes *classes,
                           struct csp_process *process, csp_id class_id)
{
    void **entry = csp_map_at(&classes->map, (uintptr_t) process);
    csp_id result = (uintptr_t) *entry;
    *entry = (void *) class_id;
    return result;
}

static void
csp_class_members_init(struct csp_class_members *members)
{
    csp_map_init(&members->map);
}

static void
csp_class_members_free_entry(void *ud, void *entry)
{
    struct csp_process_set *set = entry;
    csp_process_set_free(set);
}

static void
csp_class_members_done(struct csp_class_members *members)
{
    csp_map_done(&members->map, csp_class_members_free_entry, NULL);
}

static const struct csp_process_set *
csp_class_members_get(const struct csp_class_members *map, csp_id class_id)
{
    return csp_map_get(&map->map, class_id);
}

static bool
csp_class_members_insert(struct csp_class_members *members, csp_id class_id,
                         struct csp_process *process)
{
    struct csp_process_set **set =
            (struct csp_process_set **) csp_map_at(&members->map, class_id);
    if (unlikely(*set == NULL)) {
        *set = csp_process_set_new();
    }
    return csp_process_set_add(*set, process);
}

static bool
csp_class_members_remove(struct csp_class_members *members, csp_id class_id,
                         struct csp_process *process)
{
    struct csp_process_set *set = csp_map_get(&members->map, class_id);
    if (unlikely(set == NULL)) {
        return false;
    } else {
        return csp_process_set_remove(set, process);
    }
}

struct csp_class_members_iterator {
    struct csp_map_iterator iter;
};

static void
csp_class_members_get_iterator(const struct csp_class_members *members,
                               struct csp_class_members_iterator *iter)
{
    csp_map_get_iterator(&members->map, &iter->iter);
}

static bool
csp_class_members_iterator_done(struct csp_class_members_iterator *iter)
{
    return csp_map_iterator_done(&iter->iter);
}

static void
csp_class_members_iterator_advance(struct csp_class_members_iterator *iter)
{
    csp_map_iterator_advance(&iter->iter);
}

static csp_id
csp_class_members_iterator_get_class_id(
        const struct csp_class_members_iterator *iter)
{
    return iter->iter.key;
}

#define csp_class_members_foreach(map, iter)            \
    for (csp_class_members_get_iterator((map), (iter)); \
         !csp_class_members_iterator_done((iter));      \
         csp_class_members_iterator_advance((iter)))

struct csp_equivalences *
csp_equivalences_new(void)
{
    struct csp_equivalences *equiv = malloc(sizeof(struct csp_equivalences));
    assert(equiv != NULL);
    csp_equivalences_init(equiv);
    return equiv;
}

void
csp_equivalences_init(struct csp_equivalences *equiv)
{
    csp_class_members_init(&equiv->class_members);
    csp_process_classes_init(&equiv->process_classes);
}

void
csp_equivalences_done(struct csp_equivalences *equiv)
{
    csp_class_members_done(&equiv->class_members);
    csp_process_classes_done(&equiv->process_classes);
}

void
csp_equivalences_free(struct csp_equivalences *equiv)
{
    csp_equivalences_done(equiv);
    free(equiv);
}

void
csp_equivalences_add(struct csp_equivalences *equiv, csp_id class_id,
                     struct csp_process *process)
{
    csp_id old_class_id;

    /* First insert the process into the `process_classes` map.  As a side
     * effect, this tells us if the process was already in an equivalence class
     * (possibly this one, possibly not). */
    old_class_id = csp_process_classes_insert(&equiv->process_classes, process,
                                              class_id);

    /* If the member was already in this same equivalence class, there's nothing
     * to do. */
    if (old_class_id == class_id) {
        return;
    }

    /* If the member was already in a DIFFERENT equivalence class, remove it
     * from the old class's process set. */
    if (old_class_id != CSP_ID_NONE) {
        csp_class_members_remove(&equiv->class_members, old_class_id, process);
    }

    /* And then add the member to the new equivalence class. */
    csp_class_members_insert(&equiv->class_members, class_id, process);
}

void
csp_equivalences_build_classes(struct csp_equivalences *equiv,
                               struct csp_id_set *set)
{
    struct csp_class_members_iterator iter;
    csp_class_members_foreach (&equiv->class_members, &iter) {
        csp_id_set_add(set, csp_class_members_iterator_get_class_id(&iter));
    }
}

csp_id
csp_equivalences_get_class(struct csp_equivalences *equiv,
                           struct csp_process *process)
{
    return csp_process_classes_get(&equiv->process_classes, process);
}

const struct csp_process_set *
csp_equivalences_get_members(struct csp_equivalences *equiv, csp_id class_id)
{
    const struct csp_process_set *members =
            csp_class_members_get(&equiv->class_members, class_id);
    if (members == NULL) {
        return csp_process_set_new_empty();
    } else {
        return members;
    }
}
