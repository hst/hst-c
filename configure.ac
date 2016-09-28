# -*- coding: utf-8 -*-
# ------------------------------------------------------------------------------
# Copyright © 2016, HST Project.
# Please see the COPYING file in this distribution for license details.
# ------------------------------------------------------------------------------

AC_INIT([hst], [0.0.0], [info@hst-project.net])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/hst/hst.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz subdir-objects])
AM_MAINTAINER_MODE([enable])  # Allow packagers to disable if they want
AM_SILENT_RULES([yes])
LT_INIT

# Turn on fatal warnings by default; you can override this by setting CPPFLAGS
# to something else when running configure.
: ${CPPFLAGS="-Wall -Werror"}
AC_PROG_CC
AC_C_BIGENDIAN([AC_DEFINE([HAVE_BIG_ENDIAN], [1],
                          [Whether this system is big-endian])],
               [AC_DEFINE([HAVE_LITTLE_ENDIAN], [1],
                          [Whether this system is little-endian])])
AC_C_TYPEOF
AC_CHECK_LIB([Judy], [Judy1Set], [], [AC_MSG_ERROR([libjudy not found])])
AX_GCC_BUILTIN([__builtin_expect])
AX_GCC_FUNC_ATTRIBUTE([constructor])
AX_GCC_FUNC_ATTRIBUTE([unused])
AC_DEFINE([HAVE_BUILTIN_EXPECT], [HAVE___BUILTIN_EXPECT],
          [CCAN uses a different name for HAVE___BUILTIN_EXPECT])
AC_DEFINE([HAVE_ATTRIBUTE_UNUSED], [HAVE_FUNC_ATTRIBUTE_UNUSED],
          [CCAN uses a different name for HAVE_FUNC_ATTRIBUTE_UNUSED])

# TAP support
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AC_PROG_AWK

AC_OUTPUT([Makefile])

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  C Compiler.....: $CC $CFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
