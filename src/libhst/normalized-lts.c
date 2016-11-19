/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/compiler/compiler.h"
#include "hst.h"

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

struct csp_normalized_lts_node {
    struct csp_id_set processes;
    struct csp_behavior behavior;
    void *edges;
};

static struct csp_normalized_lts_node *
csp_normalized_lts_node_new(struct csp *csp, enum csp_semantic_model model,
                            const struct csp_id_set *processes)
{
    struct csp_normalized_lts_node *node =
            malloc(sizeof(struct csp_normalized_lts_node));
    assert(node != NULL);
    csp_id_set_init(&node->processes);
    csp_behavior_init(&node->behavior);
    csp_id_set_clone(&node->processes, processes);
    csp_process_set_get_behavior(csp, processes, model, &node->behavior);
    node->edges = NULL;
    return node;
}

static void
csp_normalized_lts_node_free(struct csp_normalized_lts_node *node)
{
    UNNEEDED Word_t dummy;
    csp_id_set_done(&node->processes);
    csp_behavior_done(&node->behavior);
    JLFA(dummy, node->edges);
    free(node);
}

static void
csp_normalized_lts_node_add_edge(struct csp_normalized_lts_node *node,
                                 csp_id event, csp_id to)
{
    Word_t *vto;
    JLI(vto, node->edges, event);
    assert(*vto == 0);
    *vto = to;
}

static csp_id
csp_normalized_lts_node_get_edge(struct csp_normalized_lts_node *node,
                                 csp_id event)
{
    Word_t *vto;
    JLG(vto, node->edges, event);
    if (vto == NULL) {
        return CSP_NODE_NONE;
    } else {
        return *vto;
    }
}

static void
csp_normalized_lts_node_get_edges(struct csp_normalized_lts_node *node,
                                  struct csp_id_pair_array *edges)
{
    Word_t count;
    Word_t *vto;
    csp_id event_id = 0;
    size_t i = 0;
    JLC(count, node->edges, 0, -1);
    csp_id_pair_array_ensure_size(edges, count);
    JLF(vto, node->edges, event_id);
    while (vto != NULL) {
        csp_id to = *vto;
        struct csp_id_pair *pair = &edges->pairs[i++];
        pair->from = event_id;
        pair->to = to;
        JLN(vto, node->edges, event_id);
    }
}

struct csp_normalized_lts {
    struct csp *csp;
    enum csp_semantic_model model;
    void *nodes;
};

struct csp_normalized_lts *
csp_normalized_lts_new(struct csp *csp, enum csp_semantic_model model)
{
    struct csp_normalized_lts *lts = malloc(sizeof(struct csp_normalized_lts));
    assert(lts != NULL);
    lts->csp = csp;
    lts->model = model;
    lts->nodes = NULL;
    return lts;
}

void
csp_normalized_lts_free(struct csp_normalized_lts *lts)
{
    {
        UNNEEDED Word_t dummy;
        Word_t *vnode;
        csp_id id = 0;
        JLF(vnode, lts->nodes, id);
        while (vnode != NULL) {
            struct csp_normalized_lts_node *node = (void *) *vnode;
            csp_normalized_lts_node_free(node);
            JLN(vnode, lts->nodes, id);
        }
        JLFA(dummy, lts->nodes);
    }
    free(lts);
}

bool
csp_normalized_lts_add_node(struct csp_normalized_lts *lts,
                            const struct csp_id_set *processes, csp_id *dest)
{
    csp_id id = processes->hash;
    Word_t *vnode;
    *dest = id;
    JLI(vnode, lts->nodes, id);
    if (*vnode == 0) {
        struct csp_normalized_lts_node *node =
                csp_normalized_lts_node_new(lts->csp, lts->model, processes);
        *vnode = (Word_t) node;
        return true;
    } else {
        return false;
    }
}

static struct csp_normalized_lts_node *
csp_normalized_lts_get_node(struct csp_normalized_lts *lts, csp_id id)
{
    Word_t *vnode;
    JLG(vnode, lts->nodes, id);
    assert(vnode != NULL);
    return (void *) *vnode;
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
                                   struct csp_id_set_builder *builder)
{
    Word_t *vnode;
    csp_id id = 0;
    JLF(vnode, lts->nodes, id);
    while (vnode != NULL) {
        csp_id_set_builder_add(builder, id);
        JLN(vnode, lts->nodes, id);
    }
}

static void
csp_normalized_lts_init_bisimulation(struct csp_normalized_lts *lts,
                                     struct csp_equivalences *equiv)
{
    /* We start by assuming that all nodes with the same behavior are
     * equivalent. */
    Word_t  *vnode;
    csp_id  id = 0;
    DEBUG("=== initialize");
    JLF(vnode, lts->nodes, id);
    while (vnode != NULL) {
        struct csp_normalized_lts_node  *node = (void *) *vnode;
        DEBUG("  init " CSP_ID_FMT " ⇒ " CSP_ID_FMT, id, node->behavior.hash);
        csp_equivalences_add(equiv, node->behavior.hash, id);
        JLN(vnode, lts->nodes, id);
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
    csp_normalized_lts_node_get_edges(from1, edges);
    assert(from1 != from2);
    DEBUG("  check " CSP_ID_FMT " ?~ " CSP_ID_FMT, id1, id2);

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
    struct csp_id_set_builder builder;
    struct csp_id_set classes;
    struct csp_id_set members;
    bool changed;

    csp_id_pair_array_init(&edges);
    csp_id_set_builder_init(&builder);
    csp_id_set_init(&classes);
    csp_id_set_init(&members);

    csp_normalized_lts_init_bisimulation(lts, equiv);
    do {
        size_t  i;
        DEBUG("=== starting new iteration");

        /* We don't want to start another iteration after this one unless we
         * find any changes. */
        changed = false;

        /* Loop through each pair of states that were equivalent before,
         * verifying that they're still equivalent.  Separate any that are not
         * equivalent to their head into a new class. */
        csp_equivalences_build_classes(equiv, &builder);
        csp_id_set_build(&classes, &builder);
        for (i = 0; i < classes.count; i++) {
            size_t  j;
            csp_id  class_id = classes.ids[i];
            csp_id  head_id;
            struct csp_normalized_lts_node  *head;
            csp_id  new_class_id = CSP_ID_NONE;
            DEBUG("class " CSP_ID_FMT, class_id);

            csp_equivalences_build_members(equiv, class_id, &builder);
            csp_id_set_build(&members, &builder);

            /* The "head" of this class is just the one that happens to be first
             * in the list of members. */
            assert(members.count > 0);
            head_id = members.ids[0];
            head = csp_normalized_lts_get_node(lts, head_id);
            DEBUG("member[0] = " CSP_ID_FMT, head_id);

            /* If we find a non-equivalent member of this class, we'll need to
             * separate it out into a new class.  This new class will need a
             * head, which will be the first non-equivalent member we find.
             *
             * If we find multiple members that aren't equivalent to the head,
             * we'll put them into the same new equivalence class; if they turn
             * out to also not be equivalent to each other, we'll catch that in
             * a later iteration. */
            for (j = 1; j < members.count; j++) {
                csp_id  member_id = members.ids[j];
                struct csp_normalized_lts_node  *member =
                    csp_normalized_lts_get_node(lts, member_id);
                DEBUG("member[%zu] = " CSP_ID_FMT, j, member_id);
                if (!csp_normalized_lts_nodes_equiv(equiv, &edges, head_id,
                                                    head, member_id, member)) {
                    /* This state is not equivalent to its head.  If necessary,
                     * create a new equivalence class.  Add the node to this new
                     * class. */
                    if (new_class_id == CSP_ID_NONE) {
                        new_class_id = member_id;
                    }
                    DEBUG("  move " CSP_ID_FMT " ⇒ " CSP_ID_FMT,
                          member_id, new_class_id);
                    csp_equivalences_add(equiv, new_class_id, member_id);
                    changed = true;
                }
            }
        }
    } while (changed);

    csp_id_pair_array_done(&edges);
    csp_id_set_builder_done(&builder);
    csp_id_set_done(&classes);
    csp_id_set_done(&members);
}
