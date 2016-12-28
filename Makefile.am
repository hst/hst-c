# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/third_party
noinst_LTLIBRARIES = libhst.la
check_LTLIBRARIES = libtests.la
check_PROGRAMS = \
	tests/test-csp0 \
	tests/test-denotational \
	tests/test-environment \
	tests/test-equivalences \
	tests/test-id-maps \
	tests/test-id-pairs \
	tests/test-id-set-maps \
	tests/test-id-sets \
	tests/test-operators \
	tests/test-refinement \
	tests/test-string-maps
bin_PROGRAMS = hst
TESTS = ${check_PROGRAMS}
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/tap-driver.sh
@VALGRIND_CHECK_RULES@

EXTRA_DIST = tap-driver.sh third_party
dist-hook:
	@rm -f `find $(top_distdir)/third_party -name $(am__dirstamp)`
	@rm -f `find $(top_distdir)/third_party -name '*.o'`
	@rm -f `find $(top_distdir)/third_party -name '*.lo'`
	@rm -rf `find $(top_distdir)/third_party -type d -name $(DEPDIR)`
	@rm -rf `find $(top_distdir)/third_party -type d -name .libs`

libhst_la_SOURCES = \
	src/basics.h \
	src/behavior.c \
	src/csp0.c \
	src/denotational.c \
	src/environment.c \
	src/equivalence.h \
	src/equivalence.c \
	src/hst.h \
	src/id-pair.c \
	src/id-map.h \
	src/id-map.c \
	src/id-set.h \
	src/id-set.c \
	src/id-set-map.h \
	src/id-set-map.c \
	src/macros.h \
	src/map.h \
	src/map.c \
	src/normalized-lts.c \
	src/refinement.c \
	src/string-map.h \
	src/string-map.c \
	src/operators/external-choice.c \
	src/operators/internal-choice.c \
	src/operators/prefix.c \
	src/operators/recursion.c \
	src/operators/sequential-composition.c \
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
	third_party/ccan/cppmagic/cppmagic.h \
	third_party/ccan/likely/likely.h

hst_SOURCES = \
	src/hst/hst.c
hst_LDADD = libhst.la

LDADD = libhst.la libtests.la
tests_test_csp0_LDFLAGS = -no-install
tests_test_denotational_LDFLAGS = -no-install
tests_test_environment_LDFLAGS = -no-install
tests_test_equivalences_LDFLAGS = -no-install
tests_test_id_maps_LDFLAGS = -no-install
tests_test_id_pairs_LDFLAGS = -no-install
tests_test_id_set_maps_LDFLAGS = -no-install
tests_test_id_sets_LDFLAGS = -no-install
tests_test_operators_LDFLAGS = -no-install
tests_test_refinement_LDFLAGS = -no-install
tests_test_string_maps_LDFLAGS = -no-install

dist_doc_DATA = README.md
