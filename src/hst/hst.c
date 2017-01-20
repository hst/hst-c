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

#include "has-trace.c.in"
#include "reachable.c.in"

struct command {
    const char *name;
    void (*run)(int argc, char **argv);
};

static struct command commands[] = {
        {"has-trace", has_trace}, {"reachable", reachable}, {NULL, NULL}};

static bool
streq(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

int
main(int argc, char **argv)
{
    const char *command;
    struct command *curr;

    if (argc <= 1) {
        fprintf(stderr, "Usage: hst [command]\n");
        exit(EXIT_FAILURE);
    }

    argc--, argv++; /* Executable name */
    command = *argv;

    for (curr = commands; curr->name != NULL; curr++) {
        if (streq(command, curr->name)) {
            curr->run(argc, argv);
            exit(EXIT_SUCCESS);
        }
    }

    fprintf(stderr, "Unknown command %s\n", command);
    exit(EXIT_FAILURE);
}
