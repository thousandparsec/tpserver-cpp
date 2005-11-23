AC_INIT([tpserver-cpp],[0.1.2],[lee@thousandparsec.net])
AC_CONFIG_SRCDIR(src/main.cpp)

AC_CANONICAL_TARGET([])

AM_INIT_AUTOMAKE(tpserver-cpp, 0.1.2)
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



dnl PLT MzScheme autoconf test
AC_ARG_WITH(mzscheme,
  AS_HELP_STRING([--with-mzscheme[=prefix]],
    [Set the prefix to plt mzscheme]
  ),
  ,
  [with_mzscheme="auto"]
)

if test "$with_mzscheme" = "no"; then
  AC_MSG_ERROR([MzScheme is required, please specific prefix with --with-mzscheme])
fi
if test "$with_mzscheme" = "auto"; then
  with_mzscheme=/usr/lib/plt
fi
if test -n "$with_mzscheme" && test -d "$with_mzscheme" ; then
  CPPFLAGS="$CPPFLAGS -I$with_mzscheme/include"
  LDFLAGS="$LDFLAGS -L$with_mzscheme/lib"
fi

AC_CHECK_LIB(dl, dlsym,,)
AC_CHECK_HEADER(scheme.h)
AC_CHECK_LIB(mzgc, GC_init,,AC_MSG_ERROR(Can not find MzScheme library, required to build))
AC_CHECK_LIB(mzscheme,scheme_basic_env,,AC_MSG_ERROR(Can not find MzScheme library, required to build))

if test -n "$with_mzscheme"; then
   AC_MSG_WARN([MzScheme is not nice, please check that $with_mzscheme/lib is in /etc/ld.so.conf or TPServer-cpp WILL NOT RUN])
fi

AM_CONDITIONAL(MZSCHEME, true)


dnl MySql autoconf test stolen from BONIC, then hacked, lots
AC_ARG_WITH(mysql,
  AS_HELP_STRING([--with-mysql],
    [Set to use mysql]
  ),
  ,
)
AC_ARG_VAR([MYSQL_CONFIG], [mysql_config program])
if test "$with_mysql" = "no"; then
    AM_CONDITIONAL(MYSQL, false)
else

    if test -z "$MYSQL_CONFIG"; then
        AC_PATH_PROG(MYSQL_CONFIG,mysql_config,,)
    fi

    # THIS SMALL CHANGE TO THE STANDARD .M4 FILE SIMPLY SETS A VARIABLE IF
    # MYSQL IS NOT THERE.
    if test -z "$MYSQL_CONFIG"; then
        no_mysql=yes
    else
        AC_MSG_CHECKING(mysql libraries)
        MYSQL_LIBS=`${MYSQL_CONFIG} --libs`
        AC_MSG_RESULT($MYSQL_LIBS)
        AC_MSG_CHECKING(mysql includes)
        MYSQL_CFLAGS=`${MYSQL_CONFIG} --cflags`
        CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS"
        AC_MSG_RESULT($MYSQL_CFLAGS)
        no_mysql=no
    fi
    AC_SUBST(MYSQL_LIBS)
    
    if test "$no_mysql" = "yes"; then
        AM_CONDITIONAL(MYSQL, false)
    else
        AC_DEFINE(HAVE_LIBMYSQL, 1, [Define to 1 if you have the `mysql' library (-lmysql).])
        AM_CONDITIONAL(MYSQL, true)
    fi
fi


dnl @synopsis AC_DEFINE_DIR(VARNAME, DIR [, DESCRIPTION])
dnl
dnl This macro _AC_DEFINEs VARNAME to the expansion of the DIR
dnl variable, taking care of fixing up ${prefix} and such.
dnl
dnl Note that the 3 argument form is only supported with autoconf 2.13 and
dnl later (i.e. only where _AC_DEFINE supports 3 arguments).
dnl
dnl Examples:
dnl
dnl    AC_DEFINE_DIR(DATADIR, datadir)
dnl    AC_DEFINE_DIR(PROG_PATH, bindir, [Location of installed binaries])
dnl
dnl @author Guido Draheim <guidod@gmx.de>, original by Alexandre Oliva

AC_DEFUN([AC_DEFINE_DIR], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_define_dir=`eval echo [$]$2`
  ac_define_dir=`eval echo [$]ac_define_dir`
  ifelse($3, ,
    AC_DEFINE_UNQUOTED($1, "$ac_define_dir"),
    AC_DEFINE_UNQUOTED($1, "$ac_define_dir", $3))
])

AC_DEFINE_DIR(DATADIR, datadir, The data directory where datafiles will be stored)

AC_CONFIG_FILES([
Makefile
src/Makefile
]) 

AC_OUTPUT
