#
# configure script for emma
#
AC_PREREQ([2.68])
AC_INIT([emma], [0.1], [lornix@lornix.com])
#
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2])
#
# prevent configure from setting CXXFLAGS=-g -O2
:${CXXFLAGS=""}

# Checks for programs.
AC_PROG_CXX

# config header file
AC_CONFIG_HEADERS([config.h])

# Checks for header files.
AC_CHECK_HEADERS(bfd.h)
AC_CHECK_HEADERS(dis-asm.h)

# Checks for libraries.
AC_CHECK_LIB([bfd], [bfd_init])
AC_CHECK_LIB([opcodes], [disassembler_usage])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
                 Makefile
                 src/Makefile
                 ])
AC_OUTPUT
