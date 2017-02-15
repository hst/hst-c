# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016-2017, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Preliminaries

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/third_party \
	-I$(top_srcdir)/third_party/judy/src
noinst_LTLIBRARIES = \
	libjudy.la \
	libjudy1.la \
	libjudy1count.la \
	libjudy1next.la \
	libjudy1prev.la \
	libjudyl.la \
	libjudylcount.la \
	libjudylnext.la \
	libjudylprev.la \
	libhst.la
noinst_PROGRAMS = \
	judy1tables \
	judyltables
check_LTLIBRARIES = libtests.la
check_PROGRAMS = \
	tests/test-bfs \
	tests/test-csp0 \
	tests/test-denotational \
	tests/test-environment \
	tests/test-equivalences \
	tests/test-events \
	tests/test-event-sets \
	tests/test-id-sets \
	tests/test-process-sets \
	tests/test-operators \
	tests/test-refinement
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

#-------------------------------------------------------------------------------
# libjudy

judy1tables_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/Judy1
judy1tables_CFLAGS = "${JUDY_CFLAGS}" -DJUDY1 -fno-strict-aliasing
judy1tables_SOURCES = \
	third_party/judy/src/JudyCommon/JudyTables.c
judy1tables_LDADD =

Judy1Tables.c: judy1tables
	${AM_V_GEN}cd $(builddir) && ./judy1tables

judyltables_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/JudyL
judyltables_CFLAGS = "${JUDY_CFLAGS}" -DJUDYL -fno-strict-aliasing
judyltables_SOURCES = \
	third_party/judy/src/JudyCommon/JudyTables.c
judyltables_LDADD =

JudyLTables.c: judyltables
	${AM_V_GEN}cd $(builddir) && ./judyltables

BUILT_SOURCES = Judy1Tables.c JudyLTables.c
CLEANFILES = Judy1Tables.c JudyLTables.c

libjudy1_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/Judy1
libjudy1_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDY1 -fno-strict-aliasing
libjudy1_la_SOURCES = \
	Judy1Tables.c \
	third_party/judy/src/JudyCommon/JudyCascade.c \
	third_party/judy/src/JudyCommon/JudyCount.c \
	third_party/judy/src/JudyCommon/JudyCreateBranch.c \
	third_party/judy/src/JudyCommon/JudyDecascade.c \
	third_party/judy/src/JudyCommon/JudyDel.c \
	third_party/judy/src/JudyCommon/JudyFirst.c \
	third_party/judy/src/JudyCommon/JudyFreeArray.c \
	third_party/judy/src/JudyCommon/JudyGet.c \
	third_party/judy/src/JudyCommon/JudyIns.c \
	third_party/judy/src/JudyCommon/JudyInsArray.c \
	third_party/judy/src/JudyCommon/JudyInsertBranch.c \
	third_party/judy/src/JudyCommon/JudyMallocIF.c \
	third_party/judy/src/JudyCommon/JudyMemActive.c \
	third_party/judy/src/JudyCommon/JudyMemUsed.c

libjudy1count_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/Judy1
libjudy1count_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDY1 -DNOSMARTJBB -DNOSMARTJBU -DNOSMARTJLB
libjudy1count_la_SOURCES = \
	third_party/judy/src/JudyCommon/JudyByCount.c
libjudy1count_la_LIBADD = libjudy1.la

libjudy1next_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/Judy1
libjudy1next_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDY1 -DJUDYNEXT
libjudy1next_la_SOURCES = \
	third_party/judy/src/JudyCommon/JudyPrevNext.c \
	third_party/judy/src/JudyCommon/JudyPrevNextEmpty.c
libjudy1next_la_LIBADD = libjudy1.la

libjudy1prev_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/Judy1
libjudy1prev_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDY1 -DJUDYPREV
libjudy1prev_la_SOURCES = \
	third_party/judy/src/JudyCommon/JudyPrevNext.c \
	third_party/judy/src/JudyCommon/JudyPrevNextEmpty.c
libjudy1prev_la_LIBADD = libjudy1.la

libjudyl_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/JudyL
libjudyl_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDYL -fno-strict-aliasing
libjudyl_la_SOURCES = \
	JudyLTables.c \
	third_party/judy/src/JudyCommon/JudyCascade.c \
	third_party/judy/src/JudyCommon/JudyCount.c \
	third_party/judy/src/JudyCommon/JudyCreateBranch.c \
	third_party/judy/src/JudyCommon/JudyDecascade.c \
	third_party/judy/src/JudyCommon/JudyDel.c \
	third_party/judy/src/JudyCommon/JudyFirst.c \
	third_party/judy/src/JudyCommon/JudyFreeArray.c \
	third_party/judy/src/JudyCommon/JudyGet.c \
	third_party/judy/src/JudyCommon/JudyInsArray.c \
	third_party/judy/src/JudyCommon/JudyIns.c \
	third_party/judy/src/JudyCommon/JudyInsertBranch.c \
	third_party/judy/src/JudyCommon/JudyMallocIF.c \
	third_party/judy/src/JudyCommon/JudyMemActive.c \
	third_party/judy/src/JudyCommon/JudyMemUsed.c

libjudylcount_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/JudyL
libjudylcount_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDYL -DNOSMARTJBB -DNOSMARTJBU -DNOSMARTJLB
libjudylcount_la_SOURCES = \
	third_party/judy/src/JudyCommon/JudyByCount.c
libjudylcount_la_LIBADD = libjudyl.la

libjudylnext_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/JudyL
libjudylnext_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDYL -DJUDYNEXT
libjudylnext_la_SOURCES = \
	third_party/judy/src/JudyCommon/JudyPrevNext.c \
	third_party/judy/src/JudyCommon/JudyPrevNextEmpty.c
libjudylnext_la_LIBADD = libjudyl.la

libjudylprev_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/third_party/judy/src/JudyCommon \
	-I$(top_srcdir)/third_party/judy/src/JudyL
libjudylprev_la_CFLAGS = "${JUDY_CFLAGS}" -DJUDYL -DJUDYPREV
libjudylprev_la_SOURCES = \
	third_party/judy/src/JudyCommon/JudyPrevNext.c \
	third_party/judy/src/JudyCommon/JudyPrevNextEmpty.c
libjudylprev_la_LIBADD = libjudyl.la

libjudy_la_SOURCES = \
	src/judy-malloc.c
libjudy_la_LIBADD = \
	libjudy1.la \
	libjudy1count.la \
	libjudy1next.la \
	libjudy1prev.la \
	libjudylcount.la \
	libjudylnext.la \
	libjudylprev.la

#-------------------------------------------------------------------------------
# HST and tests

libhst_la_SOURCES = \
	src/basics.h \
	src/behavior.h \
	src/behavior.c \
	src/csp0.h \
	src/csp0.c \
	src/denotational.h \
	src/denotational.c \
	src/environment.h \
	src/environment.c \
	src/equivalence.h \
	src/equivalence.c \
	src/event.h \
	src/event.c \
	src/id-set.h \
	src/id-set.c \
	src/macros.h \
	src/map.h \
	src/map.c \
	src/normalization.h \
	src/normalization.c \
	src/process.h \
	src/process.c \
	src/refinement.h \
	src/refinement.c \
	src/set.h \
	src/set.c \
	src/operators.h \
	src/operators/external-choice.c \
	src/operators/interleave.c \
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
libhst_la_LIBADD = libjudy.la

libtests_la_SOURCES = \
	tests/test-cases.h \
	tests/test-case-harness.h \
	third_party/ccan/compiler/compiler.h \
	third_party/ccan/cppmagic/cppmagic.h \
	third_party/ccan/likely/likely.h

hst_SOURCES = \
	src/hst/has-trace.c.in \
	src/hst/hst.c \
	src/hst/reachable.c.in \
	src/hst/traces.c.in
hst_LDADD = libhst.la

LDADD = libhst.la libtests.la
tests_test_bfs_LDFLAGS = -no-install
tests_test_csp0_LDFLAGS = -no-install
tests_test_denotational_LDFLAGS = -no-install
tests_test_environment_LDFLAGS = -no-install
tests_test_equivalences_LDFLAGS = -no-install
tests_test_events_LDFLAGS = -no-install
tests_test_event_sets_LDFLAGS = -no-install
tests_test_id_sets_LDFLAGS = -no-install
tests_test_operators_LDFLAGS = -no-install
tests_test_process_sets_LDFLAGS = -no-install
tests_test_refinement_LDFLAGS = -no-install

dist_doc_DATA = README.md
