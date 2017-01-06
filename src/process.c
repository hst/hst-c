/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include "process.h"

#include "basics.h"
#include "environment.h"
#include "id-set.h"

void
csp_process_free(struct csp *csp, struct csp_process *process)
{
    process->free(csp, process);
}

void
csp_process_build_initials(struct csp *csp, struct csp_process *process,
                           struct csp_id_set *set)
{
    process->initials(csp, process, set);
}

void
csp_process_build_afters(struct csp *csp, struct csp_process *process,
                         csp_id initial, struct csp_id_set *set)
{
    process->afters(csp, process, initial, set);
}
