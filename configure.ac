AC_INIT([tpserver-cpp],[0.0.9],[lee@thousandparsec.net])
AC_CONFIG_SRCDIR(src/main.cpp)

AC_CANONICAL_TARGET([])

AM_INIT_AUTOMAKE(tpserver-cpp, 0.0.9)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

CXXFLAGS="-g -Wall"


AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_LANG_CPLUSPLUS
AC_REQUIRE_CPP

AC_HEADER_STDC



AC_CHECK_LIB(pthread, pthread_create,,AC_MSG_ERROR(TP Server requires pthread library))
#check headers, functions, etc

AC_C_BIGENDIAN

dnl Test for IPv6 socket addresses
dnl "Borrowed" from skstream


AC_MSG_CHECKING([for IPv6 socket address])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
                #include <sys/socket.h>
                #include <netinet/in.h>]], [[
    sockaddr_in6 sin6;
    sin6.sin6_family = AF_INET6;
]])],[
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_IPV6, 1, true if we have sockaddr_in6 and AF_INET6)
],[
   AC_MSG_RESULT(no)
])



AC_CONFIG_FILES([
Makefile
src/Makefile
]) 

AC_OUTPUT
