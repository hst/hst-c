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
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccan/compiler/compiler.h"
#include "ccan/cppmagic/cppmagic.h"
#include "ccan/likely/likely.h"
#include "hst.h"

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
 * Cleaning up test cases
 */

typedef void test_case_cleanup_f(void *);

struct test_case_cleanup {
    test_case_cleanup_f  *func;
    void  *ud;
    struct test_case_cleanup  *next;
};

static struct test_case_cleanup  *cleanup_routines;

static void
test_case_cleanup_start(void)
{
    cleanup_routines = NULL;
}

UNNEEDED
static void
test_case_cleanup_register(test_case_cleanup_f *func, void *ud)
{
    struct test_case_cleanup  *routine =
        malloc(sizeof(struct test_case_cleanup));
    assert(routine != NULL);
    routine->func = func;
    routine->ud = ud;
    routine->next = cleanup_routines;
    cleanup_routines = routine;
}

static void
test_case_cleanup_finish(void)
{
    struct test_case_cleanup  *curr;
    struct test_case_cleanup  *next;
    for (curr = cleanup_routines; curr != NULL; curr = next) {
        next = curr->next;
        curr->func(curr->ud);
        free(curr);
    }
    cleanup_routines = NULL;
}

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

UNNEEDED
static void
vfail_at(const char* filename, unsigned int line, const char* fmt, va_list args)
{
    if (!test_case_failed) {
        printf("not ok %u - %s\n", test_case_number,
               current_test_case->description);
    }
    printf("# ");
    vprintf(fmt, args);
    printf("\n# at %s:%u\n", filename, line);
    test_case_failed = true;
    any_test_case_failed = true;
}

UNNEEDED
static void
fail_at(const char* filename, unsigned int line, const char* fmt, ...)
{
    va_list  args;
    va_start(args, fmt);
    vfail_at(filename, line, fmt, args);
    va_end(args);
}

static jmp_buf  test_case_return;

static void
run_test(struct test_case_descriptor *test)
{
    if (test->function == NULL) {
        printf("# %s\n", test->description);
    } else {
        test_case_cleanup_start();
        current_test_case = test;
        test_case_failed = false;
        test_case_number++;
        if (setjmp(test_case_return) == 0) {
            test->function();
        }
        test_case_cleanup_finish();
        if (likely(!test_case_failed)) {
            printf("ok %u - %s\n", test_case_number, test->description);
        }
    }
}

NORETURN
static void
abort_test(void)
{
    longjmp(test_case_return, 1);
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
 * More preprocessor nonsense
 */

/* Calls m with whatever parameter list immediately follows, but with __FILE__
 * and __LINE__ prepended to it.  For instance,
 *
 *     ADD_FILE_AND_LINE(x)(1, 2)
 *
 * expands to
 *
 *     x(__FILE__, __LINE__, 1, 2)
 */
#define ADD_FILE_AND_LINE(m)  m ADD_FILE_AND_LINE_
#define ADD_FILE_AND_LINE_(...) (__FILE__, __LINE__, __VA_ARGS__)

/* Expands to the length of __VA_ARGS__ */
#define LENGTH(...) CPPMAGIC_JOIN(+, CPPMAGIC_MAP(LENGTH1_, __VA_ARGS__))
#define LENGTH1_(x) 1

/*-----------------------------------------------------------------------------
 * Data constructors
 */

UNNEEDED
static void
csp_id_set_free_(void *vset)
{
    struct csp_id_set  *set = vset;
    csp_id_set_done(set);
    free(set);
}

/* Creates a new empty set.  The set will be automatically freed for you at the
 * end of the test case. */
UNNEEDED
static struct csp_id_set *
empty_set(void)
{
    struct csp_id_set  *set = malloc(sizeof(struct csp_id_set));
    assert(set != NULL);
    csp_id_set_init(set);
    test_case_cleanup_register(csp_id_set_free_, set);
    return set;
}

/* Creates a new set containing the given IDs.  The set will be automatically
 * freed for you at the end of the test case. */
#define id_set(...) \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
        (id_set_(LENGTH(__VA_ARGS__), __VA_ARGS__)) \
        (empty_set())

UNNEEDED
static struct csp_id_set *
id_set_(size_t count, ...)
{
    size_t  i;
    va_list  args;
    struct csp_id_set  *set = malloc(sizeof(struct csp_id_set));
    struct csp_id_set_builder  builder;
    assert(set != NULL);
    csp_id_set_init(set);
    test_case_cleanup_register(csp_id_set_free_, set);
    csp_id_set_builder_init(&builder);
    va_start(args, count);
    for (i = 0; i < count; i++) {
        csp_id  id = va_arg(args, csp_id);
        csp_id_set_builder_add(&builder, id);
    }
    va_end(args);
    csp_id_set_build(set, &builder);
    csp_id_set_builder_done(&builder);
    return set;
}

/* Creates a new set containing the given range of IDs.  The set will be
 * automatically freed for you at the end of the test case. */
UNNEEDED
static struct csp_id_set *
id_range_set(size_t start, size_t end)
{
    size_t  i;
    struct csp_id_set  *set = malloc(sizeof(struct csp_id_set));
    struct csp_id_set_builder  builder;
    assert(set != NULL);
    csp_id_set_init(set);
    test_case_cleanup_register(csp_id_set_free_, set);
    csp_id_set_builder_init(&builder);
    for (i = start; i < end; i++) {
        csp_id_set_builder_add(&builder, i);
    }
    csp_id_set_build(set, &builder);
    csp_id_set_builder_done(&builder);
    return set;
}

/*-----------------------------------------------------------------------------
 * Check macros
 */

#define fail  ADD_FILE_AND_LINE(fail_at)

#define check_alloc_with_msg(var, call, ...) \
    do { \
        var = call; \
        if (unlikely(var == NULL)) { \
            fail_at(__FILE__, __LINE__, __VA_ARGS__); \
            abort_test(); \
        } \
    } while (0)

#define check_alloc(var, call) \
    check_alloc_with_msg(var, call, "Cannot allocate " #var)

UNNEEDED
static void
check_with_msg_(const char *filename, unsigned int line, bool result,
                const char *fmt, ...)
{
    if (unlikely(!result)) {
        va_list  args;
        va_start(args, fmt);
        vfail_at(filename, line, fmt, args);
        va_end(args);
        abort_test();
    }
}
#define check_with_msg  ADD_FILE_AND_LINE(check_with_msg_)
#define check(call)  check_with_msg(call, "Error occurred")

UNNEEDED
static void
check0_with_msg_(const char *filename, unsigned int line, int result,
                 const char *fmt, ...)
{
    if (unlikely(result != 0)) {
        va_list  args;
        va_start(args, fmt);
        vfail_at(filename, line, fmt, args);
        va_end(args);
        abort_test();
    }
}
#define check0_with_msg  ADD_FILE_AND_LINE(check0_with_msg_)
#define check0(call)  check0_with_msg(call, "Error occurred")

UNNEEDED
static void
checkx0_with_msg_(const char *filename, unsigned int line, int result,
                  const char *fmt, ...)
{
    if (unlikely(result == 0)) {
        va_list  args;
        va_start(args, fmt);
        vfail_at(filename, line, fmt, args);
        va_end(args);
        abort_test();
    }
}
#define checkx0_with_msg  ADD_FILE_AND_LINE(checkx0_with_msg_)
#define checkx0(call)  checkx0_with_msg(call, "Error should have occurred")

UNNEEDED
static void
check_nonnull_with_msg_(const char *filename, unsigned int line, void *result,
                        const char *fmt, ...)
{
    if (unlikely(result == NULL)) {
        va_list  args;
        va_start(args, fmt);
        vfail_at(filename, line, fmt, args);
        va_end(args);
        abort_test();
    }
}
#define check_nonnull_with_msg  ADD_FILE_AND_LINE(check_nonnull_with_msg_)
#define check_nonnull(call)  check_nonnull_with_msg(call, "Error occurred")

UNNEEDED
static void
check_id_eq_(const char *filename, unsigned int line, csp_id id1, csp_id id2)
{
    check_with_msg_(filename, line, (id1 == id2),
            "Expected IDs to be equal, got " CSP_ID_FMT " and " CSP_ID_FMT,
            id1, id2);
}
#define check_id_eq  ADD_FILE_AND_LINE(check_id_eq_)

UNNEEDED
static void
check_id_ne_(const char *filename, unsigned int line, csp_id id1, csp_id id2)
{
    check_with_msg_(filename, line, (id1 != id2),
            "Expected IDs to be unequal, got " CSP_ID_FMT, id1);
}
#define check_id_ne  ADD_FILE_AND_LINE(check_id_ne_)

UNNEEDED
static void
check_streq_(const char *filename, unsigned int line, const char *actual,
             const char *expected)
{
    check_with_msg_(filename, line, strcmp(actual, expected) == 0,
            "Expected \"%s\", got \"%s\"", expected, actual);
}
#define check_streq  ADD_FILE_AND_LINE(check_streq_)

UNNEEDED
static void
check_set_eq_(const char *filename, unsigned int line,
              const struct csp_id_set *actual,
              const struct csp_id_set *expected)
{
    if (unlikely(!csp_id_set_eq(actual, expected))) {
        size_t  i;
        struct csp_id_set_builder  builder;
        struct csp_id_set  diff;
        fail_at(filename, line, "Expected sets to be equal");
        printf("# hash of actual   = " CSP_ID_FMT "\n", actual->hash);
        printf("# hash of expected = " CSP_ID_FMT "\n", expected->hash);
        csp_id_set_builder_init(&builder);
        csp_id_set_init(&diff);

        printf("# Elements only in actual:\n");
        csp_id_set_builder_merge(&builder, expected);
        for (i = 0; i < actual->count; i++) {
            csp_id  curr = actual->ids[i];
            if (!csp_id_set_builder_remove(&builder, curr)) {
                printf("#   " CSP_ID_FMT "\n", curr);
            }
        }
        csp_id_set_build(&diff, &builder);

        printf("# Elements only in expected:\n");
        csp_id_set_builder_merge(&builder, actual);
        for (i = 0; i < expected->count; i++) {
            csp_id  curr = expected->ids[i];
            if (!csp_id_set_builder_remove(&builder, curr)) {
                printf("#   " CSP_ID_FMT "\n", curr);
            }
        }
        csp_id_set_build(&diff, &builder);

        csp_id_set_builder_done(&builder);
        csp_id_set_done(&diff);
        abort_test();
    }
}
#define check_set_eq  ADD_FILE_AND_LINE(check_set_eq_)

/**
 * CPPMAGIC_MAP_SEMICOLONS - iterate another macro across arguments
 * @m: name of a one argument macro
 *
 * CPPMAGIC_MAP_SEMICOLONS(@m, @a1, @a2, ... @an)
 *     expands to the expansion of @m(@a1) ; @m(@a2) ; ... ; @m(@an)
 */
#define _CPPMAGIC_MAP_SEMICOLONS_()  _CPPMAGIC_MAP_SEMICOLONS
#define _CPPMAGIC_MAP_SEMICOLONS(m_, a_, ...) \
    m_(a_) \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
        (; CPPMAGIC_DEFER2(_CPPMAGIC_MAP_SEMICOLONS_)()(m_, __VA_ARGS__)) \
        ()
#define CPPMAGIC_MAP_SEMICOLONS(m_, ...) \
    CPPMAGIC_IFELSE(CPPMAGIC_NONEMPTY(__VA_ARGS__)) \
        (CPPMAGIC_EVAL(_CPPMAGIC_MAP_SEMICOLONS(m_, __VA_ARGS__))) \
        ()

/* Takes a parenthesized list and removes the parentheses. */
#define CPPMAGIC_UNPACK(args) CPPMAGIC_UNPACK_ args
#define CPPMAGIC_UNPACK_(...) __VA_ARGS__

/* Add IDs to a set.  `elements` should be a (possibly empty) parenthesized list
 * of "elements".  `id_adder` should be a macro that takes in one of those
 * elements, translates it into an ID, and adds that ID to an ID set builder
 * named `__builder`. */
#define fill_id_set(id_adder, set, elements) \
    do { \
        struct csp_id_set_builder  __builder; \
        csp_id_set_builder_init(&__builder); \
        CPPMAGIC_MAP_SEMICOLONS(id_adder, CPPMAGIC_UNPACK(elements)); \
        csp_id_set_build((set), &__builder); \
        csp_id_set_builder_done(&__builder); \
    } while (0)

/* Adds events to a set.  `events` should be a (possibly empty) parenthesized
 * list of event names.  We automatically translate those event names into event
 * IDs. */
#define fill_event_id_set(set, events) \
    fill_id_set(add_event_id, set, events)
#define add_event_id(event_name) \
    do { \
        csp_id  __event = csp_get_event_id(csp, (event_name)); \
        csp_id_set_builder_add(&__builder, __event); \
    } while (0)

/* Adds processes to a cset.  `processes` should be a (possibly empty)
 * parenthesized list of CSP₀ process descriptions.  We automatically parse
 * those into process IDs and add those processes to `set`. */
#define fill_csp0_set(set, processes) \
    fill_id_set(add_csp0_process, set, processes)
#define add_csp0_process(str) \
    do { \
        csp_id  __process; \
        check0(csp_load_csp0_string(csp, (str), &__process)); \
        csp_id_set_builder_add(&__builder, __process); \
    } while (0)

#endif /* TEST_CASES_H */
