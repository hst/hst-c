/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef TEST_CASE_HARNESS_H
#define TEST_CASE_HARNESS_H

#include "test-cases.h"

/*-----------------------------------------------------------------------------
 * Harness
 */

int
main(void)
{
    run_tests();
    return exit_status();
}

#endif /* TEST_CASE_HARNESS_H */
