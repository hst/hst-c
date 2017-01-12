/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_EQUIVALENCE_H
#define HST_EQUIVALENCE_H

#include "id-map.h"
#include "id-set-map.h"

/* Stores information about the "equivalence classes" of a set of processes.
 * All of the processes that have "equivalent" behavior (according to one of
 * CSP's semantic models) belong to the same equivalence class. */

struct csp_equivalences {
    struct csp_id_set_map classes;
    struct csp_id_map members;
};

struct csp_equivalences *
csp_equivalences_new(void);

void
csp_equivalences_init(struct csp_equivalences *equiv);

void
csp_equivalences_done(struct csp_equivalences *equiv);

void
csp_equivalences_free(struct csp_equivalences *equiv);

/* Add a member to an equivalence class.  If the member was already in an
 * equivalence class, it is removed from that one before adding it to the new
 * one. */
void
csp_equivalences_add(struct csp_equivalences *equiv, csp_id class_id,
                     csp_id member_id);

/* Add the IDs of all of the equivalence classes to a set. */
void
csp_equivalences_build_classes(struct csp_equivalences *equiv,
                               struct csp_id_set *set);

/* Return the class that a member belongs to, or CSP_ID_NONE if that member
 * hasn't been added to an equivalence class yet. */
csp_id
csp_equivalences_get_class(struct csp_equivalences *equiv, csp_id member_id);

/* Add all of the members of an equivalence class to a set. */
void
csp_equivalences_build_members(struct csp_equivalences *equiv, csp_id class_id,
                               struct csp_id_set *set);

#endif /* HST_EQUIVALENCE_H */
