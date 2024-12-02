#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# If you change this file, you usually should run
# autoreconf -fvi
AC_PREREQ(2.61)

AC_INIT([DADA],[1.0],[andrew.jameson@gmail.com])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])

AC_DISABLE_SHARED

# Automake initialization
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX_11([noext], [optional])
AM_CONDITIONAL([HAVE_CXX11], [test x"$HAVE_CXX11" == x1])

#
# Checks for header files.
#
AC_HEADER_STDC

#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T

#
# Checks for library functions.
#
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF

AC_CONFIG_FILES([Makefile
  src/Makefile])

AC_OUTPUT
