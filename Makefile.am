# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I. -I$(top_srcdir)/include -I$(top_srcdir)/third_party
include_HEADERS = include/hst.h
lib_LTLIBRARIES = libhst.la
check_PROGRAMS = \
	tests/test-environment \
	tests/test-id-sets
bin_PROGRAMS = hst
TESTS = ${check_PROGRAMS}

LDADD = libhst.la

libhst_la_SOURCES = \
	third_party/ccan/hash/hash.c \
	src/libhst/environment.c \
	src/libhst/id-set.c
libhst_la_LDFLAGS = -export-symbols-regex '^csp_[^_]'

tests_test_environment_LDFLAGS = -no-install
tests_test_id_sets_LDFLAGS = -no-install

hst_SOURCES = \
	src/hst/hst.c

dist_doc_DATA = README
