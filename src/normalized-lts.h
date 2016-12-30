/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_NORMALIZED_LTS_H
#define HST_NORMALIZED_LTS_H

#include "basics.h"
#include "behavior.h"
#include "environment.h"
#include "equivalence.h"
#include "id-pair.h"
#include "id-set.h"

/*------------------------------------------------------------------------------
 * Normalized LTS
 */

#define CSP_NODE_NONE CSP_ID_NONE

struct csp_normalized_lts;

struct csp_normalized_lts *
csp_normalized_lts_new(struct csp *csp, enum csp_semantic_model model);

void
csp_normalized_lts_free(struct csp_normalized_lts *lts);

/* Create a new normalized LTS node for the given set of processes, if it
 * doesn't already exist.  Returns whether a new node was created.  Places the
 * ID of the node into `id` either way.  Takes control of the content of
 * `processes`; it will be cleared upon return. */
bool
csp_normalized_lts_add_node(struct csp_normalized_lts *lts,
                            struct csp_id_set *processes, csp_id *id);

/* Adds a new edge to a normalized LTS.  `from` and `to` must the IDs of nodes
 * that you've already created via csp_normalized_lts_add_node.  `event` must be
 * an event ID.  If there is already an edge with the same `from` and `event`,
 * it must also have the same `to`. */
void
csp_normalized_lts_add_edge(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event, csp_id to);

/* Add a "normalized root".  During a refinement check, this lets us keep track
 * of which normalized LTS node your Spec process belongs to.  (You won't have
 * to call this function directly; we'll keep track of this for you for each
 * process that you prenormalize.) */
void
csp_normalized_lts_add_normalized_root(struct csp_normalized_lts *lts,
                                       csp_id root_id,
                                       csp_id normalized_root_id);

/* Return the normalized LTS node that a particular original non-normalized
 * process belongs to.  (That is, if you start a refinement check for a
 * particular Spec process, which normalized LTS node should you start the
 * breadth-first search from?) */
csp_id
csp_normalized_lts_get_normalized_root(struct csp_normalized_lts *lts,
                                       csp_id root_id);

/* Return the behavior for a normalized LTS node.  `id` must a node that you've
 * already created via csp_normalized_lts_add_node.  We retain ownership of the
 * returned behavior. */
const struct csp_behavior *
csp_normalized_lts_get_node_behavior(struct csp_normalized_lts *lts, csp_id id);

/* Return the outgoing edges for a normalized LTS node.  `id` must a node that
 * you've already created via csp_normalized_lts_add_node. */
void
csp_normalized_lts_get_node_edges(struct csp_normalized_lts *lts, csp_id id,
                                  struct csp_id_pair_array *edges);

/* Return the set of processes for a normalized LTS node.  `id` must a node that
 * you've already created via csp_normalized_lts_add_node.  We retain ownership
 * of the returned set. */
const struct csp_id_set *
csp_normalized_lts_get_node_processes(struct csp_normalized_lts *lts,
                                      csp_id id);

/* Return the normalized node that can be reached by following `event` from the
 * given `from` process.  Returns CSP_NODE_NONE if there is no such node. */
csp_id
csp_normalized_lts_get_edge(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event);

/* Add the IDs of all of the normalized nodes in `lts` to `set`. */
void
csp_normalized_lts_build_all_nodes(struct csp_normalized_lts *lts,
                                   struct csp_id_set *set);

/* Bisimulate all of the nodes in a normalized LTS, to find nodes that have
 * equivalent behavior.  Each node in the normalized LTS will have an entry in
 * `equivalence`; the value of each entry will be the "representative node" for
 * the equivalence class that the node belongs to.  All nodes in the same
 * equivalence class will have the same representative node. */
void
csp_normalized_lts_bisimulate(struct csp_normalized_lts *lts,
                              struct csp_equivalences *equiv);

/* Merge together equivalent nodes in the normalized LTS, according to the given
 * equivalence relation, placing the result into a new normalized LTS. */
void
csp_normalized_lts_merge_equivalences(struct csp_normalized_lts *dest,
                                      struct csp_equivalences *equiv);

#endif /* HST_NORMALIZED_LTS_H */
