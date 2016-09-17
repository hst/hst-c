/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef TEST_CASES_H
#define TEST_CASES_H

#include "config.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

struct test_case {
    struct test_case  *next;
    void (*function)(void);
    const char  *description;
};

static struct test_case  *test_cases = NULL;
static unsigned int  test_case_count = 0;


#define TEST_CASE(description)           TEST_CASE_AT(description, __LINE__)
#define TEST_CASE_AT(description, line)  TEST_CASE_AT_(description, line)
#define TEST_CASE_AT_(description, line) \
static void \
test_case__##line(void); \
\
static struct test_case test_case_##line = { \
    NULL, \
    test_case__##line, \
    description \
}; \
\
CONSTRUCTOR \
static void \
register_test_case_##line(void) \
{ \
    test_case_##line.next = test_cases; \
    test_cases = &test_case_##line; \
    test_case_count++; \
} \
\
static void \
test_case__##line(void)


#define TEST_CASE_GROUP(desc)           TEST_CASE_GROUP_AT(desc, __LINE__)
#define TEST_CASE_GROUP_AT(desc, line)  TEST_CASE_GROUP_AT_(desc, line)
#define TEST_CASE_GROUP_AT_(description, line) \
static struct test_case test_case_group_##line = { \
    NULL, \
    NULL, \
    description \
}; \
\
CONSTRUCTOR \
static void \
register_test_case_group_##line(void) \
{ \
    test_case_group_##line.next = test_cases; \
    test_cases = &test_case_group_##line; \
}


static struct test_case  *current_test_case;
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
run_test(struct test_case *test)
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

/* Reverses the list of test cases.  (The registration functions pushed the test
 * cases into a singly-linked list, which puts them into reverse order.  Calling
 * this puts them back into the same order as they appear in the source.) */
static void
reverse_test_cases(void)
{
    struct test_case  *head = NULL;
    struct test_case  *curr;
    struct test_case  *next;
    for (curr = test_cases; curr != NULL; curr = next) {
        next = curr->next;
        curr->next = head;
        head = curr;
    }
    test_cases = head;
}

static void
run_tests(void)
{
    struct test_case  *curr;
    reverse_test_cases();
    printf("1..%u\n", test_case_count);
    test_case_number = 0;
    any_test_case_failed = false;
    for (curr = test_cases; curr != NULL; curr = curr->next) {
        run_test(curr);
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


#endif /* TEST_CASES_H */
