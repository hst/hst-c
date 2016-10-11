# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I. -I$(top_srcdir)/include -I$(top_srcdir)/third_party
include_HEADERS = include/hst.h
lib_LTLIBRARIES = libhst.la
check_LTLIBRARIES = libtests.la
check_PROGRAMS = \
	tests/test-environment \
	tests/test-id-sets \
	tests/test-operators
bin_PROGRAMS = hst
TESTS = ${check_PROGRAMS}
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	     $(top_srcdir)/build-aux/tap-driver.sh
@VALGRIND_CHECK_RULES@

EXTRA_DIST = third_party

libhst_la_SOURCES = \
	src/libhst/environment.c \
	src/libhst/id-set.c \
	src/libhst/operators/external-choice.c \
	src/libhst/operators/internal-choice.c \
	src/libhst/operators/prefix.c \
	third_party/ccan/build_assert/build_assert.h \
	third_party/ccan/compiler/compiler.h \
	third_party/ccan/container_of/container_of.h \
	third_party/ccan/hash/hash.h \
	third_party/ccan/hash/hash.c \
	third_party/ccan/likely/likely.h
libhst_la_LDFLAGS = -export-symbols-regex '^csp_[^_]'

libtests_la_SOURCES = \
	tests/test-cases.h \
	tests/test-case-harness.h \
	third_party/ccan/compiler/compiler.h \
	third_party/ccan/likely/likely.h

hst_SOURCES = \
	src/hst/hst.c
hst_LDADD = libhst.la

LDADD = libhst.la libtests.la
tests_test_environment_LDFLAGS = -no-install
tests_test_id_sets_LDFLAGS = -no-install
tests_test_operators_LDFLAGS = -no-install

dist_doc_DATA = README
