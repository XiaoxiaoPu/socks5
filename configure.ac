#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([socks5], [0.1.0], [https://github.com/XiaoxiaoPu/socks5/issues], [socks5], [https://github.com/XiaoxiaoPu/socks5])
AC_CONFIG_SRCDIR([socks5])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_AUX_DIR(.)
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_INSTALL

# Checks for libraries.
LIB_ANL="-lanl"
LIB_EV="-lev"
AC_CHECK_LIB([anl], [getaddrinfo_a], [AC_SUBST(LIB_ANL)])
AC_CHECK_LIB([ev], [ev_default_loop], [AC_SUBST(LIB_EV)], [libev=no])

# Checks for header files.
AC_HEADER_ASSERT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([bzero socket strchr strdup strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
