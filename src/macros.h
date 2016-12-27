/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#ifndef HST_MACROS_H
#define HST_MACROS_H

#define swap(a, b)        \
    do {                  \
        typeof(a) __swap; \
        __swap = (a);     \
        (a) = (b);        \
        (b) = __swap;     \
    } while (0)

#endif /* HST_MACROS_H */
