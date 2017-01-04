/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_EVENT_H
#define HST_EVENT_H

#include <stdlib.h>

#include "ccan/compiler/compiler.h"
#include "basics.h"

struct csp_event;

CONST_FUNCTION
const struct csp_event *
csp_tau(void);

CONST_FUNCTION
const struct csp_event *
csp_tick(void);

/* Return the ID of the event with the given name.  `name` does not need to be
 * NUL-terminated, but it cannot contain any NULs.  If you call this multiple
 * times with the same name, you'll get the same result each time. */
PURE_FUNCTION
const struct csp_event *
csp_event_get(const char *name);

PURE_FUNCTION
const struct csp_event *
csp_event_get_sized(const char *name, size_t name_length);

PURE_FUNCTION
csp_id
csp_event_id(const struct csp_event *event);

PURE_FUNCTION
const char *
csp_event_name(const struct csp_event *event);

#endif /* HST_EVENT_H */
