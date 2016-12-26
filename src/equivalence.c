/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define JUDYERROR_NOTEST 1
#include <Judy.h>

#include "ccan/build_assert/build_assert.h"
#include "ccan/compiler/compiler.h"
#include "ccan/likely/likely.h"
#include "hst.h"

void
csp_equivalences_init(struct csp_equivalences *equiv)
{
    equiv->classes = NULL;
    equiv->members = NULL;
}

void
csp_equivalences_done(struct csp_equivalences *equiv)
{
    UNNEEDED Word_t dummy;
    Word_t *vmembers;
    csp_id class_id = 0;
    JLF(vmembers, equiv->classes, class_id);
    while (vmembers != NULL) {
        void *members = (void *) *vmembers;
        J1FA(dummy, members);
        JLN(vmembers, equiv->classes, class_id);
    }
    JLFA(dummy, equiv->classes);
    JLFA(dummy, equiv->members);
}

void
csp_equivalences_add(struct csp_equivalences *equiv, csp_id class_id,
                     csp_id member_id)
{
    Word_t *vclass;
    Word_t old_class_id;
    Word_t *vmembers;
    void **members;
    UNNEEDED int rc;

    /* First insert the member_id into the `members` map.  As a side effect,
     * this tells us if the member was already in an equivalence class. */
    JLI(vclass, equiv->members, member_id);
    old_class_id = *vclass;
    *vclass = class_id;

    /* If the member was already in this same equivalence class, there's nothing
     * to do. */
    if (old_class_id == class_id) {
        return;
    }

    /* If the member was already in a DIFFERENT equivalence class, remove it. */
    if (old_class_id != 0) {
        JLG(vmembers, equiv->classes, old_class_id);
        assert(vmembers != NULL);
        members = (void **) vmembers;
        J1U(rc, *members, member_id);
    }

    /* And then add the member to the new equivalence class. */
    JLI(vmembers, equiv->classes, class_id);
    members = (void **) vmembers;
    J1S(rc, *members, member_id);
}

void
csp_equivalences_build_classes(struct csp_equivalences *equiv,
                               struct csp_id_set *set)
{
    Word_t *vmembers;
    csp_id class_id = 0;
    JLF(vmembers, equiv->classes, class_id);
    while (vmembers != NULL) {
        csp_id_set_add(set, class_id);
        JLN(vmembers, equiv->classes, class_id);
    }
}

csp_id
csp_equivalences_get_class(struct csp_equivalences *equiv, csp_id member_id)
{
    Word_t *vclass;
    JLG(vclass, equiv->members, member_id);
    if (vclass == NULL) {
        return CSP_ID_NONE;
    } else {
        return *vclass;
    }
}

void
csp_equivalences_build_members(struct csp_equivalences *equiv, csp_id class_id,
                               struct csp_id_set *set)
{
    Word_t *vmembers;
    JLG(vmembers, equiv->classes, class_id);
    if (vmembers != NULL) {
        void **members = (void **) vmembers;
        int found;
        csp_id member_id = 0;
        J1F(found, *members, member_id);
        while (found) {
            csp_id_set_add(set, member_id);
            J1N(found, *members, member_id);
        }
    }
}
