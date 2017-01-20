/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_CSP0_H
#define HST_CSP0_H

#include "basics.h"
#include "denotational.h"
#include "environment.h"

/*------------------------------------------------------------------------------
 * CSP₀
 */

/* Load in a CSP₀ process from an in-memory string.  If the CSP₀ process is
 * invalid, returns NULL. */
struct csp_process *
csp_load_csp0_string(struct csp *csp, const char *str);

/* Load in a trace from an in-memory string.  If the trace is invalid, returns
 * -1.  You must free the trace when you're done with it using
 * csp_trace_free_deep. */
int
csp_load_trace_string(struct csp *csp, const char *str,
                      struct csp_trace **dest);

#endif /* HST_CSP0_H */
