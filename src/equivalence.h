/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_EQUIVALENCE_H
#define HST_EQUIVALENCE_H

#include "id-set.h"
#include "map.h"
#include "process.h"

/* Stores information about the "equivalence classes" of a set of processes.
 * All of the processes that have "equivalent" behavior (according to one of
 * CSP's semantic models) belong to the same equivalence class. */

/* Maps a class ID to a process set. */
struct csp_class_members {
    struct csp_map map;
};

/* Maps a process to its class ID. */
struct csp_process_classes {
    struct csp_map map;
};

struct csp_equivalences {
    struct csp_class_members class_members;
    struct csp_process_classes process_classes;
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
                     struct csp_process *process);

/* Add the IDs of all of the equivalence classes to a set. */
void
csp_equivalences_build_classes(struct csp_equivalences *equiv,
                               struct csp_id_set *set);

/* Return the class that a member belongs to, or CSP_ID_NONE if that member
 * hasn't been added to an equivalence class yet. */
csp_id
csp_equivalences_get_class(struct csp_equivalences *equiv,
                           struct csp_process *process);

/* Returns the set of all of the members in an equivalence class. */
const struct csp_process_set *
csp_equivalences_get_members(struct csp_equivalences *equiv, csp_id class_id);

#endif /* HST_EQUIVALENCE_H */
