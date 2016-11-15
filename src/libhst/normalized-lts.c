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

struct csp_normalized_lts_node {
    struct csp_id_set processes;
    void *edges;
};

static struct csp_normalized_lts_node *
csp_normalized_lts_node_new(const struct csp_id_set *processes)
{
    struct csp_normalized_lts_node *node =
            malloc(sizeof(struct csp_normalized_lts_node));
    assert(node != NULL);
    csp_id_set_init(&node->processes);
    csp_id_set_clone(&node->processes, processes);
    node->edges = NULL;
    return node;
}

static void
csp_normalized_lts_node_free(struct csp_normalized_lts_node *node)
{
    UNNEEDED Word_t dummy;
    csp_id_set_done(&node->processes);
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

struct csp_normalized_lts {
    void *nodes;
};

struct csp_normalized_lts *
csp_normalized_lts_new(struct csp *csp)
{
    struct csp_normalized_lts *lts = malloc(sizeof(struct csp_normalized_lts));
    assert(lts != NULL);
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
                csp_normalized_lts_node_new(processes);
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
