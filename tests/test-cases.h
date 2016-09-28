/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef TEST_CASES_H
#define TEST_CASES_H

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/compiler/compiler.h"
#include "ccan/likely/likely.h"

/*------------------------------------------------------------------------------
 * Compiler attributes
 */

/* Declare that a function should be automatically run before main() when the
 * program starts.  This is used below to auto-register our test cases.
 *
 * TODO: Support non-GCC attributes here. */
#if HAVE_FUNC_ATTRIBUTE_CONSTRUCTOR
#define CONSTRUCTOR __attribute__((__constructor__))
#else
#error "Need __attribute__((__constructor__))"
#endif

/*------------------------------------------------------------------------------
 * Test cases
 */

#define MAX_TEST_CASE_COUNT  100

/* Each descriptor defines either a test case or a test comment.  For a test
 * comment, we just display the description as a comment, and do nothing else.
 * For a test case, we call the test case function, which should use the fail
 * and check_* macros below to signal whether the test succeeds or fails.  If it
 * succeeds, we use the description in the "ok" TAP output.  If it fails, the
 * macros let you provide a more specific description of what exactly failed; we
 * print the test case's description as a comment. */

struct test_case_descriptor {
    struct test_case_descriptor  *next;
    void (*function)(void);
    const char  *description;
    unsigned int  line;
};

static struct test_case_descriptor  *test_cases[MAX_TEST_CASE_COUNT];
static unsigned int  test_case_descriptor_count = 0;

static void
register_test_case(struct test_case_descriptor *test)
{
    assert(test_case_descriptor_count < MAX_TEST_CASE_COUNT);
    test_cases[test_case_descriptor_count++] = test;
}


#define TEST_CASE(description)           TEST_CASE_AT(description, __LINE__)
#define TEST_CASE_AT(description, line)  TEST_CASE_AT_(description, line)
#define TEST_CASE_AT_(description, line) \
static void \
test_case__##line(void); \
\
static struct test_case_descriptor test_case_##line = { \
    NULL, \
    test_case__##line, \
    description, \
    line \
}; \
\
CONSTRUCTOR \
static void \
register_test_case_##line(void) \
{ \
    register_test_case(&test_case_##line); \
} \
\
static void \
test_case__##line(void)


#define TEST_CASE_GROUP(desc)           TEST_CASE_GROUP_AT(desc, __LINE__)
#define TEST_CASE_GROUP_AT(desc, line)  TEST_CASE_GROUP_AT_(desc, line)
#define TEST_CASE_GROUP_AT_(description, line) \
static struct test_case_descriptor test_case_group_##line = { \
    NULL, \
    NULL, \
    description, \
    line \
}; \
\
CONSTRUCTOR \
static void \
register_test_case_group_##line(void) \
{ \
    register_test_case(&test_case_group_##line); \
}


static struct test_case_descriptor  *current_test_case;
static unsigned int  test_case_number;
static bool  test_case_failed;
static bool  any_test_case_failed;

static void
fail_at(const char* filename, unsigned int line, const char* fmt, ...)
{
    va_list args;
    if (!test_case_failed) {
        printf("not ok %u - %s\n", test_case_number,
               current_test_case->description);
    }
    printf("# ");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n# at %s:%u\n", filename, line);
    test_case_failed = true;
    any_test_case_failed = true;
}

static void
run_test(struct test_case_descriptor *test)
{
    if (test->function == NULL) {
        printf("# %s\n", test->description);
    } else {
        current_test_case = test;
        test_case_failed = false;
        test_case_number++;
        test->function();
        if (likely(!test_case_failed)) {
            printf("ok %u - %s\n", test_case_number, test->description);
        }
    }
}

static int
compare_test_cases(const void *vtest1, const void *vtest2)
{
    const struct test_case_descriptor * const  *test1 = vtest1;
    const struct test_case_descriptor * const  *test2 = vtest2;
    if ((*test1)->line < (*test2)->line) {
        return -1;
    } else if ((*test1)->line == (*test2)->line) {
        return 0;
    } else {
        return 1;
    }
}

static unsigned int
count_test_cases(void)
{
    unsigned int  i;
    unsigned int  count = 0;
    for (i = 0; i < test_case_descriptor_count; i++) {
        if (test_cases[i]->function != NULL) {
            count++;
        }
    }
    return count;
}

static void
run_tests(void)
{
    unsigned int  i;
    qsort(test_cases, test_case_descriptor_count,
          sizeof(struct test_case_descriptor *), compare_test_cases);
    printf("1..%u\n", count_test_cases());
    test_case_number = 0;
    any_test_case_failed = false;
    for (i = 0; i < test_case_descriptor_count; i++) {
        run_test(test_cases[i]);
    }
}

static int
exit_status(void)
{
    return any_test_case_failed? 1: 0;
}

/*-----------------------------------------------------------------------------
 * Helper macros
 */

#define fail(...)  fail_at(__FILE__, __LINE__, __VA_ARGS__)

#define check_alloc_with_msg(var, call, ...) \
    do { \
        var = call; \
        if (unlikely(var == NULL)) { \
            fail_at(__FILE__, __LINE__, __VA_ARGS__); \
            return; \
        } \
    } while (0)

#define check_alloc(var, call) \
    check_alloc_with_msg(var, call, "Cannot allocate " #var)

#define check_with_msg(call, ...) \
    do { \
        if (unlikely(!(call))) { \
            fail_at(__FILE__, __LINE__, __VA_ARGS__); \
            return; \
        } \
    } while (0)

#define check(call)  check_with_msg(call, "Error occurred")

#define check0_with_msg(call, ...) \
    do { \
        if (unlikely((call) != 0)) { \
            fail_at(__FILE__, __LINE__, __VA_ARGS__); \
            return; \
        } \
    } while (0)

#define check0(call)  check0_with_msg(call, "Error occurred")

#define checkx0_with_msg(call, ...) \
    do { \
        if (unlikely((call) == 0)) { \
            fail_at(__FILE__, __LINE__, __VA_ARGS__); \
            return; \
        } \
    } while (0)

#define checkx0(call)  checkx0_with_msg(call, "Error should have occurred")

#define check_nonnull_with_msg(call, ...) \
    do { \
        if (unlikely((call) == NULL)) { \
            fail_at(__FILE__, __LINE__, __VA_ARGS__); \
            return; \
        } \
    } while (0)

#define check_nonnull(call)  check_nonnull_with_msg(call, "Error occurred")

#define check_id_eq(id1, id2) \
    check_with_msg((id1) == (id2), \
            "Expected IDs to be equal, got 0x%08lx and 0x%08lx", \
            (id1), (id2))

#define check_id_ne(id1, id2) \
    check_with_msg((id1) != (id2), \
            "Expected IDs to be unequal, got 0x%08lx", (id1))

#define check_streq(actual, expected) \
    check_with_msg(strcmp((actual), (expected)) == 0, \
            "Expected \"%s\", got \"%s\"", (expected), (actual))

#define build_set(set, ...) \
    do { \
        csp_id  __to_add[] = { __VA_ARGS__ }; \
        size_t  __count = sizeof(__to_add) / sizeof(__to_add[0]); \
        struct csp_id_set_builder  builder; \
        csp_id_set_builder_init(&builder); \
        csp_id_set_builder_add_many(&builder, __count, __to_add); \
        csp_id_set_build((set), &builder); \
        csp_id_set_builder_done(&builder); \
    } while (0)

#define check_set_size(set, expected) \
    check_with_msg((set).count == (expected), \
            "Expected set to have size %zu, got %zu", \
            (size_t) (expected), (set).count)

#define check_set_empty_msg(msg, set) \
    check_with_msg((set).count == 0, (msg))

UNNEEDED
static int
compare_ids(const void *vid1, const void *vid2)
{
    const csp_id  *id1 = vid1;
    const csp_id  *id2 = vid2;
    if (*id1 < *id2) {
        return -1;
    } else if (*id1 == *id2) {
        return 0;
    } else {
        return 1;
    }
}

#define check_set_elements(set, ...) \
    do { \
        csp_id  __expected[] = { __VA_ARGS__ }; \
        size_t  __count = sizeof(__expected) / sizeof(__expected[0]); \
        size_t  __i; \
        /* Sort the expected elements before comparing */ \
        qsort(__expected, __count, sizeof(csp_id), compare_ids); \
        for (__i = 0; __i < __count; __i++) { \
            check_with_msg((set).ids[__i] == __expected[__i], \
                    "Expected set[%zu] to be %lu, got %lu", \
                    __i, __expected[__i], (set).ids[__i]); \
        } \
    } while (0)

#define check_set_elements_msg(msg, set, ...) \
    do { \
        csp_id  __expected[] = { __VA_ARGS__ }; \
        size_t  __count = sizeof(__expected) / sizeof(__expected[0]); \
        size_t  __i; \
        /* Sort the expected elements before comparing */ \
        qsort(__expected, __count, sizeof(csp_id), compare_ids); \
        for (__i = 0; __i < __count; __i++) { \
            check_with_msg((set).ids[__i] == __expected[__i], \
                    "%s: Expected set[%zu] to be %lu, got %lu", \
                    (msg), __i, __expected[__i], (set).ids[__i]); \
        } \
    } while (0)

#define check_set_range(set, count) \
    do { \
        check_set_size(set, count); \
        size_t  __i; \
        for (__i = 0; __i < (count); __i++) { \
            check_with_msg((set).ids[__i] == __i, \
                    "Expected set[%zu] to be %zu, got %lu", \
                    __i, __i, (set).ids[__i]); \
        } \
    } while (0)


#endif /* TEST_CASES_H */
