/* -*- coding: utf-8 -*-
 * -----------------------------------------------------------------------------
 * Copyright © 2016-2017, HST Project.
 * Please see the COPYING file in this distribution for license details.
 * -----------------------------------------------------------------------------
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reachable.c.in"

static bool
streq(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

int
main(int argc, char **argv)
{
    const char *command;

    if (argc <= 1) {
        fprintf(stderr, "Usage: hst [command]\n");
        exit(EXIT_FAILURE);
    }

    argc--, argv++; /* Executable name */
    command = *argv;

    if (streq(command, "reachable")) {
        reachable(argc, argv);
    } else {
        fprintf(stderr, "Unknown command %s\n", command);
        exit(EXIT_FAILURE);
    }

    return 0;
}
