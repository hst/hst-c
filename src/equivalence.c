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

#include "id-map.h"
#include "id-set-map.h"
#include "id-set.h"

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
    csp_id_set_map_init(&equiv->classes);
    csp_id_map_init(&equiv->members);
}

void
csp_equivalences_done(struct csp_equivalences *equiv)
{
    csp_id_set_map_done(&equiv->classes);
    csp_id_map_done(&equiv->members);
}

void
csp_equivalences_free(struct csp_equivalences *equiv)
{
    csp_equivalences_done(equiv);
    free(equiv);
}

void
csp_equivalences_add(struct csp_equivalences *equiv, csp_id class_id,
                     csp_id member_id)
{
    csp_id old_class_id;

    /* First insert the member_id into the `members` map.  As a side effect,
     * this tells us if the member was already in an equivalence class. */
    old_class_id = csp_id_map_insert(&equiv->members, member_id, class_id);

    /* If the member was already in this same equivalence class, there's nothing
     * to do. */
    if (old_class_id == class_id) {
        return;
    }

    /* If the member was already in a DIFFERENT equivalence class, remove it. */
    if (old_class_id != 0) {
        csp_id_set_map_remove(&equiv->classes, old_class_id, member_id);
    }

    /* And then add the member to the new equivalence class. */
    csp_id_set_map_insert(&equiv->classes, class_id, member_id);
}

void
csp_equivalences_build_classes(struct csp_equivalences *equiv,
                               struct csp_id_set *set)
{
    struct csp_id_set_map_iterator iter;
    csp_id_set_map_foreach (&equiv->classes, &iter) {
        csp_id_set_add(set, csp_id_set_map_iterator_get_from(&iter));
    }
}

csp_id
csp_equivalences_get_class(struct csp_equivalences *equiv, csp_id member_id)
{
    return csp_id_map_get(&equiv->members, member_id);
}

void
csp_equivalences_build_members(struct csp_equivalences *equiv, csp_id class_id,
                               struct csp_id_set *set)
{
    const struct csp_id_set *members =
            csp_id_set_map_get(&equiv->classes, class_id);
    if (members != NULL) {
        csp_id_set_union(set, members);
    }
}
