/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "normalized-lts.h"

#include <assert.h>
#include <stdlib.h>

#include "ccan/compiler/compiler.h"
#include "basics.h"
#include "id-map.h"
#include "macros.h"
#include "map.h"

#if defined(REFINEMENT_DEBUG)
#include <stdio.h>
#define DEBUG(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while (0)
#else
#define DEBUG(...)   /* do nothing */
#endif

/*------------------------------------------------------------------------------
 * Normalized LTS node
 */

struct csp_normalized_lts_node {
    struct csp_id_set processes;
    struct csp_behavior behavior;
    struct csp_id_map edges;
};

static struct csp_normalized_lts_node *
csp_normalized_lts_node_new(struct csp *csp, enum csp_semantic_model model,
                            struct csp_id_set *processes)
{
    struct csp_normalized_lts_node *node =
            malloc(sizeof(struct csp_normalized_lts_node));
    assert(node != NULL);
    node->processes = *processes;
    csp_id_set_init(processes);
    csp_behavior_init(&node->behavior);
    csp_process_set_get_behavior(csp, &node->processes, model, &node->behavior);
    csp_id_map_init(&node->edges);
    return node;
}

static void
csp_normalized_lts_node_free(struct csp_normalized_lts_node *node)
{
    csp_id_set_done(&node->processes);
    csp_behavior_done(&node->behavior);
    csp_id_map_done(&node->edges);
    free(node);
}

static void
csp_normalized_lts_node_add_edge(struct csp_normalized_lts_node *node,
                                 csp_id event, csp_id to)
{
    csp_id_map_insert(&node->edges, event, to);
}

static csp_id
csp_normalized_lts_node_get_edge(struct csp_normalized_lts_node *node,
                                 csp_id event)
{
    return csp_id_map_get(&node->edges, event);
}

static void
csp_normalized_lts_node_get_edges(struct csp_normalized_lts_node *node,
                                  struct csp_id_pair_array *edges)
{
    size_t i = 0;
    struct csp_id_map_iterator iter;
    size_t count = csp_id_map_size(&node->edges);
    csp_id_pair_array_ensure_size(edges, count);
    csp_id_map_foreach (&node->edges, &iter) {
        struct csp_id_pair *pair = &edges->pairs[i++];
        pair->from = csp_id_map_iterator_get_from(&iter);
        pair->to = csp_id_map_iterator_get_to(&iter);
    }
}

/*------------------------------------------------------------------------------
 * ID→normalized node map
 */

struct csp_normalized_lts_nodes {
    struct csp_map map;
};

void
csp_normalized_lts_nodes_init(struct csp_normalized_lts_nodes *nodes)
{
    csp_map_init(&nodes->map);
}

static void
csp_normalized_lts_nodes_free_entry(void *ud, void *entry)
{
    struct csp_normalized_lts_node *node = entry;
    csp_normalized_lts_node_free(node);
}

void
csp_normalized_lts_nodes_done(struct csp_normalized_lts_nodes *nodes)
{
    csp_map_done(&nodes->map, csp_normalized_lts_nodes_free_entry, NULL);
}

struct csp_normalized_lts_node **
csp_normalized_lts_nodes_at(struct csp_normalized_lts_nodes *nodes, csp_id id)
{
    return (struct csp_normalized_lts_node **) csp_map_at(&nodes->map, id);
}

struct csp_normalized_lts_node *
csp_normalized_lts_nodes_get(const struct csp_normalized_lts_nodes *nodes,
                             csp_id id)
{
    return csp_map_get(&nodes->map, id);
}

struct csp_normalized_lts_nodes_iterator {
    struct csp_map_iterator iter;
};

void
csp_normalized_lts_nodes_get_iterator(
        const struct csp_normalized_lts_nodes *nodes,
        struct csp_normalized_lts_nodes_iterator *iter)
{
    csp_map_get_iterator(&nodes->map, &iter->iter);
}

bool
csp_normalized_lts_nodes_iterator_done(
        struct csp_normalized_lts_nodes_iterator *iter)
{
    return csp_map_iterator_done(&iter->iter);
}

void
csp_normalized_lts_nodes_iterator_advance(
        struct csp_normalized_lts_nodes_iterator *iter)
{
    csp_map_iterator_advance(&iter->iter);
}

csp_id
csp_normalized_lts_nodes_iterator_get_key(
        const struct csp_normalized_lts_nodes_iterator *iter)
{
    return iter->iter.key;
}

struct csp_normalized_lts_node *
csp_normalized_lts_nodes_iterator_get_value(
        const struct csp_normalized_lts_nodes_iterator *iter)
{
    return *iter->iter.value;
}

#define csp_normalized_lts_nodes_foreach(nodes, iter)            \
    for (csp_normalized_lts_nodes_get_iterator((nodes), (iter)); \
         !csp_normalized_lts_nodes_iterator_done((iter));        \
         csp_normalized_lts_nodes_iterator_advance((iter)))

/*------------------------------------------------------------------------------
 * Normalized LTS
 */

struct csp_normalized_lts {
    struct csp *csp;
    enum csp_semantic_model model;
    struct csp_equivalences roots;
    struct csp_normalized_lts_nodes nodes;
};

struct csp_normalized_lts *
csp_normalized_lts_new(struct csp *csp, enum csp_semantic_model model)
{
    struct csp_normalized_lts *lts = malloc(sizeof(struct csp_normalized_lts));
    assert(lts != NULL);
    lts->csp = csp;
    lts->model = model;
    csp_equivalences_init(&lts->roots);
    csp_normalized_lts_nodes_init(&lts->nodes);
    return lts;
}

void
csp_normalized_lts_free(struct csp_normalized_lts *lts)
{
    csp_normalized_lts_nodes_done(&lts->nodes);
    csp_equivalences_done(&lts->roots);
    free(lts);
}

bool
csp_normalized_lts_add_node(struct csp_normalized_lts *lts,
                            struct csp_id_set *processes, csp_id *dest)
{
    csp_id id = csp_id_set_hash(processes);
    struct csp_normalized_lts_node **node =
            csp_normalized_lts_nodes_at(&lts->nodes, id);
    *dest = id;
    if (*node == NULL) {
        *node = csp_normalized_lts_node_new(lts->csp, lts->model, processes);
        return true;
    } else {
        return false;
    }
}

static struct csp_normalized_lts_node *
csp_normalized_lts_get_node(struct csp_normalized_lts *lts, csp_id id)
{
    struct csp_normalized_lts_node *node =
            csp_normalized_lts_nodes_get(&lts->nodes, id);
    assert(node != NULL);
    return node;
}

void
csp_normalized_lts_add_edge(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event, csp_id to)
{
    struct csp_normalized_lts_node *from_node;
    from_node = csp_normalized_lts_get_node(lts, from);
    assert(from_node != NULL);
    assert(csp_normalized_lts_get_node(lts, to) != NULL);
    csp_normalized_lts_node_add_edge(from_node, event, to);
}

void
csp_normalized_lts_add_normalized_root(struct csp_normalized_lts *lts,
                                       csp_id root_id,
                                       csp_id normalized_root_id)
{
    csp_equivalences_add(&lts->roots, normalized_root_id, root_id);
}

csp_id
csp_normalized_lts_get_normalized_root(struct csp_normalized_lts *lts,
                                       csp_id root_id)
{
    return csp_equivalences_get_class(&lts->roots, root_id);
}

const struct csp_behavior *
csp_normalized_lts_get_node_behavior(struct csp_normalized_lts *lts, csp_id id)
{
    struct csp_normalized_lts_node *node = csp_normalized_lts_get_node(lts, id);
    return &node->behavior;
}

void
csp_normalized_lts_get_node_edges(struct csp_normalized_lts *lts, csp_id id,
                                  struct csp_id_pair_array *edges)
{
    struct csp_normalized_lts_node *node = csp_normalized_lts_get_node(lts, id);
    csp_normalized_lts_node_get_edges(node, edges);
}

const struct csp_id_set *
csp_normalized_lts_get_node_processes(struct csp_normalized_lts *lts, csp_id id)
{
    struct csp_normalized_lts_node *node = csp_normalized_lts_get_node(lts, id);
    return &node->processes;
}

csp_id
csp_normalized_lts_get_edge(struct csp_normalized_lts *lts, csp_id from,
                            csp_id event)
{
    struct csp_normalized_lts_node *from_node;
    from_node = csp_normalized_lts_get_node(lts, from);
    assert(from_node != NULL);
    return csp_normalized_lts_node_get_edge(from_node, event);
}

void
csp_normalized_lts_build_all_nodes(struct csp_normalized_lts *lts,
                                   struct csp_id_set *set)
{
    struct csp_normalized_lts_nodes_iterator iter;
    csp_normalized_lts_nodes_foreach (&lts->nodes, &iter) {
        csp_id id = csp_normalized_lts_nodes_iterator_get_key(&iter);
        csp_id_set_add(set, id);
    }
}

static void
csp_normalized_lts_init_bisimulation(struct csp_normalized_lts *lts,
                                     struct csp_equivalences *equiv)
{
    /* We start by assuming that all nodes with the same behavior are
     * equivalent. */
    struct csp_normalized_lts_nodes_iterator iter;
    DEBUG("=== initialize");
    csp_normalized_lts_nodes_foreach (&lts->nodes, &iter) {
        csp_id id = csp_normalized_lts_nodes_iterator_get_key(&iter);
        struct csp_normalized_lts_node *node =
                csp_normalized_lts_nodes_iterator_get_value(&iter);
        DEBUG("  init " CSP_ID_FMT " ⇒ " CSP_ID_FMT, id, node->behavior.hash);
        csp_equivalences_add(equiv, node->behavior.hash, id);
    }
}

static bool
csp_equivalences_equiv(struct csp_equivalences *equiv, csp_id n1, csp_id n2)
{
    csp_id  class1 = csp_equivalences_get_class(equiv, n1);
    csp_id  class2 = csp_equivalences_get_class(equiv, n2);
    assert(class1 != CSP_ID_NONE);
    assert(class2 != CSP_ID_NONE);
    DEBUG("      " CSP_ID_FMT " ∈ " CSP_ID_FMT, n1, class1);
    DEBUG("      " CSP_ID_FMT " ∈ " CSP_ID_FMT, n2, class2);
    return class1 == class2;
}

/* Two nodes (which we assume are currently equivalent) should continue to be
 * equivalent if all of the targets from both lead to states that are themselves
 * equivalent. */
static bool
csp_normalized_lts_nodes_equiv(struct csp_equivalences *equiv,
                               struct csp_id_pair_array *edges, csp_id id1,
                               struct csp_normalized_lts_node *from1,
                               csp_id id2,
                               struct csp_normalized_lts_node *from2)
{
    size_t i;
    assert(from1 != from2);
    DEBUG("  check " CSP_ID_FMT " ?~ " CSP_ID_FMT, id1, id2);
    csp_normalized_lts_node_get_edges(from1, edges);

    /* Loop through all of node1's outgoing edges, finding the corresponding
     * outgoing edge from node2. */
    for (i = 0; i < edges->count; i++) {
        csp_id event_id = edges->pairs[i].from;
        csp_id to1 = edges->pairs[i].to;
        csp_id to2 = csp_normalized_lts_node_get_edge(from2, event_id);
        DEBUG("    --- " CSP_ID_FMT, event_id);
        DEBUG("    " CSP_ID_FMT " -" CSP_ID_FMT "→ " CSP_ID_FMT,
              id1, event_id, to1);
        DEBUG("    " CSP_ID_FMT " -" CSP_ID_FMT "→ " CSP_ID_FMT,
              id2, event_id, to2);
        /* If to1 and to2 are not equivalent, then from1 and from2 should no
         * longer be equivalent. */
        if (!csp_equivalences_equiv(equiv, to1, to2)) {
            DEBUG("  NOT EQUIVALENT");
            return false;
        }
    }

    /* We didn't find any negation, so from1 and from2 are still equivalent. */
    DEBUG("  EQUIVALENT");
    return true;
}

void
csp_normalized_lts_bisimulate(struct csp_normalized_lts *lts,
                              struct csp_equivalences *equiv)
{
    struct csp_id_pair_array edges;
    struct csp_id_set classes;
    struct csp_id_set members;
    struct csp_equivalences new_equiv;
    struct csp_equivalences *prev_equiv;
    struct csp_equivalences *next_equiv;
    bool changed;

    csp_id_pair_array_init(&edges);
    csp_id_set_init(&classes);
    csp_id_set_init(&members);
    csp_equivalences_init(&new_equiv);

    csp_normalized_lts_init_bisimulation(lts, equiv);
    prev_equiv = equiv;
    next_equiv = &new_equiv;
    do {
        struct csp_id_set_iterator i;
        DEBUG("=== starting new iteration");

        /* We don't want to start another iteration after this one unless we
         * find any changes. */
        changed = false;

        /* Loop through each pair of states that were equivalent before,
         * verifying that they're still equivalent.  Separate any that are not
         * equivalent to their head into a new class. */
        csp_id_set_clear(&classes);
        csp_equivalences_build_classes(prev_equiv, &classes);
        csp_id_set_foreach (&classes, &i) {
            struct csp_id_set_iterator j;
            size_t j_index;
            csp_id class_id = csp_id_set_iterator_get(&i);
            csp_id head_id;
            struct csp_normalized_lts_node *head;
            csp_id new_class_id = CSP_ID_NONE;
            DEBUG("class " CSP_ID_FMT, class_id);

            csp_id_set_clear(&members);
            csp_equivalences_build_members(prev_equiv, class_id, &members);

            /* The "head" of this class is just the one that happens to be first
             * in the list of members. */
            csp_id_set_get_iterator(&members, &j);
            assert(!csp_id_set_iterator_done(&j));
            head_id = csp_id_set_iterator_get(&j);
            head = csp_normalized_lts_get_node(lts, head_id);
            DEBUG("member[0] = " CSP_ID_FMT, head_id);
            csp_equivalences_add(next_equiv, class_id, head_id);

            /* If we find a non-equivalent member of this class, we'll need to
             * separate it out into a new class.  This new class will need a
             * head, which will be the first non-equivalent member we find.
             *
             * If we find multiple members that aren't equivalent to the head,
             * we'll put them into the same new equivalence class; if they turn
             * out to also not be equivalent to each other, we'll catch that in
             * a later iteration. */
            for (csp_id_set_iterator_advance(&j), j_index = 1;
                 !csp_id_set_iterator_done(&j);
                 csp_id_set_iterator_advance(&j), j_index++) {
                csp_id member_id = csp_id_set_iterator_get(&j);
                struct csp_normalized_lts_node *member =
                        csp_normalized_lts_get_node(lts, member_id);
                DEBUG("member[%zu] = " CSP_ID_FMT, j_index, member_id);
                if (!csp_normalized_lts_nodes_equiv(prev_equiv, &edges, head_id,
                                                    head, member_id, member)) {
                    /* This state is not equivalent to its head.  If necessary,
                     * create a new equivalence class.  Add the node to this new
                     * class. */
                    if (new_class_id == CSP_ID_NONE) {
                        new_class_id = member_id;
                    }
                    DEBUG("  move " CSP_ID_FMT " ⇒ " CSP_ID_FMT, member_id,
                          new_class_id);
                    csp_equivalences_add(next_equiv, new_class_id, member_id);
                    changed = true;
                } else {
                    DEBUG("  keep " CSP_ID_FMT " ⇒ " CSP_ID_FMT, member_id,
                          class_id);
                    csp_equivalences_add(next_equiv, class_id, member_id);
                }
            }
        }

        swap(prev_equiv, next_equiv);
    } while (changed);

    csp_id_pair_array_done(&edges);
    csp_id_set_done(&classes);
    csp_id_set_done(&members);
    csp_equivalences_done(&new_equiv);
}

void
csp_normalized_lts_merge_equivalences(struct csp_normalized_lts *lts,
                                      struct csp_equivalences *equiv)
{
    struct csp_normalized_lts *new_lts =
            csp_normalized_lts_new(lts->csp, lts->model);
    struct csp_id_set classes;
    struct csp_id_set members;
    struct csp_id_set normalized_members;
    struct csp_id_pair_array edges;
    struct csp_equivalences new_nodes;
    struct csp_id_set_iterator i;

    csp_id_set_init(&classes);
    csp_id_set_init(&members);
    csp_id_set_init(&normalized_members);
    csp_id_pair_array_init(&edges);
    csp_equivalences_init(&new_nodes);

    /* Loop through all of the equivalence classes, creating a new normalized
     * node for each one. */
    DEBUG("Create new nodes for each equivalence class");
    csp_id_set_clear(&classes);
    csp_equivalences_build_classes(equiv, &classes);
    csp_id_set_foreach (&classes, &i) {
        struct csp_id_set_iterator j;
        csp_id class_id = csp_id_set_iterator_get(&i);
        csp_id new_node_id;
        DEBUG("  Class " CSP_ID_FMT, class_id);
        /* We want to create a single new normalized node for this equivalence
         * class.  To do this, we merge together the processes that belong to
         * any of the original normalized nodes in the equivalence class. */
        csp_id_set_clear(&members);
        csp_equivalences_build_members(equiv, class_id, &members);
        csp_id_set_foreach (&members, &j) {
            csp_id member_id = csp_id_set_iterator_get(&j);
            const struct csp_id_set *member_processes =
                    csp_normalized_lts_get_node_processes(lts, member_id);
            DEBUG("    Member " CSP_ID_FMT, member_id);
            csp_id_set_union(&normalized_members, member_processes);
        }
        /* Create the new normalized node representing the union of all of the
         * original nodes. */
        csp_normalized_lts_add_node(new_lts, &normalized_members, &new_node_id);
        csp_equivalences_add(&new_nodes, new_node_id, class_id);
        DEBUG("    NEW NODE " CSP_ID_FMT, new_node_id);
    }

    /* Then loop through all of the edges in the original normalized LTS, adding
     * an equivalent edge in the new normalized LTS. */
    DEBUG("Add edges");
    csp_id_set_foreach (&classes, &i) {
        struct csp_id_set_iterator j;
        csp_id class_id = csp_id_set_iterator_get(&i);
        csp_id new_class_node_id =
                csp_equivalences_get_class(&new_nodes, class_id);
        assert(new_class_node_id != CSP_ID_NONE);
        DEBUG("  Class " CSP_ID_FMT " (new node " CSP_ID_FMT ")", class_id,
              new_class_node_id);
        csp_id_set_clear(&members);
        csp_equivalences_build_members(equiv, class_id, &members);
        csp_id_set_foreach (&members, &j) {
            size_t k;
            csp_id from_id = csp_id_set_iterator_get(&j);
            DEBUG("    Member " CSP_ID_FMT, from_id);
            csp_normalized_lts_get_node_edges(lts, from_id, &edges);
            for (k = 0; k < edges.count; k++) {
                csp_id event_id = edges.pairs[k].from;
                csp_id to_id = edges.pairs[k].to;
                csp_id to_class_id = csp_equivalences_get_class(equiv, to_id);
                csp_id new_to_node_id =
                        csp_equivalences_get_class(&new_nodes, to_class_id);
                assert(new_to_node_id != CSP_ID_NONE);
                DEBUG("      New edge " CSP_ID_FMT " -" CSP_ID_FMT
                      "→ " CSP_ID_FMT,
                      new_class_node_id, event_id, new_to_node_id);
                csp_normalized_lts_add_edge(new_lts, new_class_node_id,
                                            event_id, new_to_node_id);
            }
        }
    }

    /* Figure out the new normalized root for each root node. */
    DEBUG("Update normalized roots");
    csp_id_set_clear(&classes);
    csp_equivalences_build_classes(&lts->roots, &classes);
    csp_id_set_foreach (&classes, &i) {
        struct csp_id_set_iterator j;
        csp_id old_normalized_root_id = csp_id_set_iterator_get(&i);
        csp_id class_id =
                csp_equivalences_get_class(equiv, old_normalized_root_id);
        csp_id new_normalized_root_id =
                csp_equivalences_get_class(&new_nodes, class_id);
        csp_id_set_clear(&members);
        csp_equivalences_build_members(&lts->roots, old_normalized_root_id,
                                       &members);
        csp_id_set_foreach (&members, &j) {
            csp_id root_id = csp_id_set_iterator_get(&j);
            DEBUG("  Old root " CSP_ID_FMT " for " CSP_ID_FMT,
                  old_normalized_root_id, root_id);
            DEBUG("  New root " CSP_ID_FMT " for " CSP_ID_FMT,
                  new_normalized_root_id, root_id);
            csp_equivalences_add(&new_lts->roots, new_normalized_root_id,
                                 root_id);
        }
    }

    /* Swap the contents of the new LTS into the in/out parameter that the
     * caller provided. */
    swap(*lts, *new_lts);

    /* Clean up. */
    csp_normalized_lts_free(new_lts);
    csp_id_set_done(&classes);
    csp_id_set_done(&members);
    csp_id_set_done(&normalized_members);
    csp_id_pair_array_done(&edges);
    csp_equivalences_done(&new_nodes);
}
