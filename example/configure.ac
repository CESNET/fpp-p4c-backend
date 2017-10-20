AC_PREREQ([2.69])
AC_INIT([fpp-example], [1.0], [-])
AC_CONFIG_SRCDIR([parser.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign silent-rules subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([pcap], [pcap_next_ex])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdint.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
