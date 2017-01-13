/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_CSP0_H
#define HST_CSP0_H

#include "basics.h"
#include "environment.h"

/*------------------------------------------------------------------------------
 * CSP₀
 */

/* Load in a CSP₀ process from an in-memory string, placing the ID of the new
 * process in `dest`.  Returns 0 on success.  If the CSP₀ process is invalid,
 * returns -1. */
struct csp_process *
csp_load_csp0_string(struct csp *csp, const char *str);

#endif /* HST_CSP0_H */
