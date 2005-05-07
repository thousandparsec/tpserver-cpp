AC_INIT([tpserver-cpp],[0.1.0],[lee@thousandparsec.net])
AC_CONFIG_SRCDIR(src/main.cpp)

AC_CANONICAL_TARGET([])

AM_INIT_AUTOMAKE(tpserver-cpp, 0.1.0)
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

#checking for readline
READLINE_LIBS=

AC_CHECK_LIB(termcap,tgetent, 
    [READLINE_LIBS="$READLINE_LIBS -ltermcap"],
    AC_CHECK_LIB(ncurses, tgetent, 
        [READLINE_LIBS="$READLINE_LIBS -lncurses"],
        AC_MSG_ERROR([Cannot find tgetent in termcap or ncurses libraries!])
    )
)

AC_CHECK_HEADERS(termios.h unistd.h signal.h)
AC_CHECK_LIB(readline,readline,
    [
	AC_DEFINE(HAVE_LIBREADLINE, 1, true if we have libreadline)
	READLINE_LIBS="$READLINE_LIBS -lreadline"
    ],
    [
        AC_MSG_ERROR([Cannot find readline!])
    ],[ $READLINE_LIBS ]
)

    
AC_TRY_COMPILE(
  [
    #include <stdio.h>
    #include <readline/readline.h>
  ],
    readline("test > "), 
    AC_DEFINE(READLINE_CXX_SANE, 1, [Define if readline headers are C++ aware]),
    AC_MSG_WARN(Readline C++ workaround enabled)
)

AC_SUBST(READLINE_LIBS)

#check byte order
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
