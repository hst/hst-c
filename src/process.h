/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_PROCESS_H
#define HST_PROCESS_H

#include "basics.h"
#include "id-set.h"

/*------------------------------------------------------------------------------
 * Processes
 */

struct csp;

struct csp_process {
    csp_id id;

    void (*initials)(struct csp *csp, struct csp_process *process,
                     struct csp_id_set *set);

    void (*afters)(struct csp *csp, struct csp_process *process, csp_id initial,
                   struct csp_id_set *set);

    void (*free)(struct csp *csp, struct csp_process *process);
};

void
csp_process_free(struct csp *csp, struct csp_process *process);

void
csp_process_build_initials(struct csp *csp, struct csp_process *process,
                           struct csp_id_set *set);

void
csp_process_build_afters(struct csp *csp, struct csp_process *process,
                         csp_id initial, struct csp_id_set *set);

#endif /* HST_PROCESS_H */
