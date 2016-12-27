/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_BASICS_H
#define HST_BASICS_H

#include <inttypes.h>
#include <stdint.h>

/* Each process and event is identified by a number. */
typedef uint64_t csp_id;
#define CSP_ID_FMT "0x%016" PRIx64
#define CSP_ID_NONE ((csp_id) 0)

#endif /* HST_BASICS_H */
