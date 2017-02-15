/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "process.h"

#include "ccan/container_of/container_of.h"
#include "environment.h"
#include "event.h"
#include "test-case-harness.h"

struct test_visitor {
    struct csp_process_visitor visitor;
    size_t process_count;
};

/* A silly process visitor that prunes and aborts in ways that we can test. */
static int
test_visitor_visit(struct csp *csp, struct csp_process_visitor *visitor,
                   struct csp_process *process)
{
    struct test_visitor *self =
            container_of(visitor, struct test_visitor, visitor);
    struct csp_contains_event contains;

    /* Count this process regardless of its initials. */
    self->process_count++;

    /* If the process has `b` as an initial, don't consider ANY outgoing
     * transitions. */
    contains = csp_contains_event(csp_event_get("b"));
    csp_process_visit_initials(csp, process, &contains.visitor);
    if (contains.is_present) {
        return CSP_PROCESS_BFS_PRUNE;
    }

    /* If the process has `c` as an initial, abort the entire BFS. */
    contains = csp_contains_event(csp_event_get("c"));
    csp_process_visit_initials(csp, process, &contains.visitor);
    if (contains.is_present) {
        return CSP_PROCESS_BFS_ABORT;
    }

    /* Otherwise continue. */
    return CSP_PROCESS_BFS_CONTINUE;
}

static struct test_visitor
test_visitor(void)
{
    struct test_visitor self = {{test_visitor_visit}, 0};
    return self;
}

static void
check_bfs_(const char *filename, unsigned int line,
           struct csp_process_factory process_, size_t expected_count)
{
    struct csp *csp;
    struct csp_process *process;
    struct test_visitor visitor;
    check_alloc(csp, csp_new());
    process = csp_process_factory_create(csp, process_);
    visitor = test_visitor();
    csp_process_bfs(csp, process, &visitor.visitor);
    check_with_msg_(filename, line, visitor.process_count == expected_count,
                    "Unexpected process count: got %zu, expected %zu",
                    visitor.process_count, expected_count);
    csp_free(csp);
}
#define check_bfs ADD_FILE_AND_LINE(check_bfs_)

TEST_CASE_GROUP("breadth-first searches");

TEST_CASE("breadth-first searches") {
    check_bfs(csp0("STOP"), 1);
    check_bfs(csp0("a → STOP"), 2);
    check_bfs(csp0("a → a → STOP"), 3);
    check_bfs(csp0("b → STOP"), 1);
    check_bfs(csp0("c → STOP"), 1);
    check_bfs(csp0("a → STOP □ b → STOP"), 1);
    check_bfs(csp0("a → STOP □ d → STOP"), 2);
}
