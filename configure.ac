#
# configure script for emma
#
AC_PREREQ([2.68])
#
# determine current version number, from hg rev #
# if 'hg' exists, determine current tip revision
#   otherwise, use 'UNK' as revision number
define([LORNIX_VER],m4_esyscmd_s([cat VERSION]))
define([LORNIX_REV],m4_esyscmd_s([hg tip --template "{rev}"]))
define([LORNIX_REV],m4_default_nblank(LORNIX_REV,[UNK]))
define([LORNIX_VERSION],m4_join([],LORNIX_VER,[-r],LORNIX_REV))
#
# now set the name, version and contact details
AC_INIT([emma], LORNIX_VERSION, [lornix@lornix.com])
#
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2 no-dist-gzip])
#
# are we in right place?
AC_CONFIG_SRCDIR([src/Makefile.am])
#
# config header file
AC_CONFIG_HEADERS([config.h])
#
# prevent configure from setting CXXFLAGS=-g -O2
:${CXXFLAGS=""}
#
# Checks for programs.
AC_PROG_CXX
#
# Checks for libraries.
AC_CHECK_LIB([bfd], [bfd_init],,
             [AC_MSG_ERROR([Missing libbfd.*, binutils installed?])])
AC_CHECK_LIB([opcodes], [disassembler_usage],,
             [AC_MSG_ERROR([Missing libbfd.*, binutils installed?])])
#
# Checks for header files.
AC_CHECK_HEADERS([bfd.h],,
                 [AC_MSG_ERROR([Missing bfd.h, binutils-dev installed?])])
AC_CHECK_HEADERS([dis-asm.h],,
                 [AC_MSG_ERROR([Missing dis-asm.h, binutils-dev installed?])])
#
# Checks for typedefs, structures, and compiler characteristics.
#
# Checks for library functions.
#
# update these files using automake
AC_CONFIG_FILES([
                 Makefile
                 src/Makefile
                 ])
#
# write it out!
AC_OUTPUT
