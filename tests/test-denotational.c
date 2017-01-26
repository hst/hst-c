/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "denotational.h"

#include "ccan/container_of/container_of.h"
#include "csp0.h"
#include "event.h"
#include "environment.h"
#include "test-case-harness.h"

#define MAX_NAME_LENGTH 4096

struct csp_count_trace_length {
    struct csp_trace_event_visitor visitor;
    size_t length;
};

static void
csp_count_trace_length_visit(struct csp *csp,
                             struct csp_trace_event_visitor *visitor,
                             const struct csp_trace *trace, size_t index)
{
    struct csp_count_trace_length *self =
            container_of(visitor, struct csp_count_trace_length, visitor);
    if (!csp_trace_empty(trace)) {
        self->length++;
    }
}

static struct csp_count_trace_length
csp_count_trace_length(void)
{
    struct csp_count_trace_length self = {{csp_count_trace_length_visit}, 0};
    return self;
}

struct csp_collect_string {
    struct csp_name_visitor visitor;
    char str[MAX_NAME_LENGTH];
    char *cursor;
    size_t bytes_remaining;
};

static void
csp_collect_string_visit(struct csp *csp, struct csp_name_visitor *visitor,
                         const char *str, size_t length)
{
    struct csp_collect_string *self =
            container_of(visitor, struct csp_collect_string, visitor);
    assert(length < self->bytes_remaining);
    memcpy(self->cursor, str, length);
    self->cursor += length;
    self->bytes_remaining -= length;
    *self->cursor = '\0';
}

static void
csp_collect_string_init(struct csp_collect_string *self)
{
    self->visitor.visit = csp_collect_string_visit;
    self->cursor = self->str;
    self->bytes_remaining = MAX_NAME_LENGTH;
}

static void
check_trace_length_(const char *filename, unsigned int line,
                    struct csp_trace_factory trace_, size_t expected_length)
{
    struct csp *csp;
    struct csp_trace *trace;
    struct csp_count_trace_length count;
    check_alloc(csp, csp_new());
    trace = csp_trace_factory_create(csp, trace_);
    count = csp_count_trace_length();
    csp_trace_visit_events(csp, trace, &count.visitor);
    check_with_msg_(filename, line, count.length == expected_length,
                    "Unexpected trace length: got %zu, expected %zu",
                    count.length, expected_length);
    csp_free(csp);
}
#define check_trace_length ADD_FILE_AND_LINE(check_trace_length_)

static void
check_trace_print_(const char *filename, unsigned int line,
                   struct csp_trace_factory trace_, const char *expected_print)
{
    struct csp *csp;
    struct csp_trace *trace;
    struct csp_collect_string collect;
    check_alloc(csp, csp_new());
    trace = csp_trace_factory_create(csp, trace_);
    csp_collect_string_init(&collect);
    csp_trace_print(csp, trace, &collect.visitor);
    check_streq_(filename, line, collect.str, expected_print);
    csp_free(csp);
}
#define check_trace_print ADD_FILE_AND_LINE(check_trace_print_)

TEST_CASE_GROUP("traces");

TEST_CASE("can create empty trace via factory")
{
    check_trace_length(trace(), 0);
    check_trace_print(trace(), "⟨⟩");
}

TEST_CASE("can create 1-element trace via factory")
{
    check_trace_length(trace("a"), 1);
    check_trace_print(trace("a"), "⟨a⟩");
}

TEST_CASE("can create 5-element trace via factory")
{
    check_trace_length(trace("a", "b", "c", "d", "e"), 5);
    check_trace_print(trace("a", "b", "c", "d", "e"), "⟨a,b,c,d,e⟩");
}
