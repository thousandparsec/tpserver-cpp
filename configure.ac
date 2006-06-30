AC_INIT([tpserver-cpp],[0.2.1],[lee@thousandparsec.net])
AC_CONFIG_SRCDIR(tpserver/main.cpp)

AC_CANONICAL_TARGET([])

AM_INIT_AUTOMAKE(tpserver-cpp, 0.2.1)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

CXXFLAGS="-g -Wall"


AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

AC_LANG_CPLUSPLUS
AC_REQUIRE_CPP

AC_HEADER_STDC



AC_CHECK_LIB(pthread, pthread_create,,AC_MSG_ERROR(TP Server requires pthread library))
#check headers, functions, etc

AC_CHECK_LIB(dl, dlsym,,)

PKG_CHECK_MODULES(libtprl, libtprl >= 0.0.0,
	[
        CXXFLAGS="$CXXFLAGS $libtprl_CFLAGS"
        LDFLAGS="$LDFLAGS $libtprl_LIBS"
	],
	AC_MSG_ERROR(Couldn't find libtprl. Please install it before trying again)
)


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

AM_PATH_LIBGNUTLS(1.2.0, [AC_DEFINE(HAVE_LIBGNUTLS, 1, [Define to 1 if you have the `gnutls' library (-lgnutls).])
        AM_CONDITIONAL(GNUTLS, true)], AM_CONDITIONAL(GNUTLS, false))


# Always build minisec
MODULE_LIBS="-L../modules/games/minisec -lminisec"


# ===================================================
# Scheme Interpreter tests
# ===================================================

# enable the --with-mzscheme option
AC_ARG_WITH(mzscheme,
	    AS_HELP_STRING([--with-mzscheme[=path]],[Set the path to plt mzscheme]),
	    [mzscheme_option="$withval"],
	    [mzscheme_option="auto"])

# enable the --with-guile option
AC_ARG_WITH(guile,
	    AS_HELP_STRING([--with-guile[=path]],[Enable use of guile, with optional path.]),
	    [guile_option="$withval"],
	    [guile_option="auto"])



if test "$mzscheme_option" != "no"; then
   mzscheme_loc=/usr/lib/plt
   if test -n "$mzscheme_option" && test -d "$mzscheme_option" ; then
      mzscheme_loc=$mzscheme_option
   fi
   CPPFLAGS="$CPPFLAGS -I$mzscheme_loc/include"
   LDFLAGS="$LDFLAGS -L$mzscheme_loc/lib"

   AC_CHECK_HEADER(scheme.h)
   AC_DEFINE(HAVE_MZSCHEME, 1, [Defined to 1 if MzScheme libraries are available])

   # Test for MZScheme 2.x vs. 3.x
   AC_MSG_CHECKING(mzscheme version)
   AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include "scheme.h"
	#ifndef SCHEME_STRINGP
	#error
	#endif]])],
	[AC_DEFINE(HAVE_MZSCHEME20X, 1,Define if we have mzscheme 209)
        AC_MSG_RESULT(20x)],
        AC_MSG_RESULT(3xx))

   AC_CHECK_LIB(mzgc, GC_init,,)
   AC_CHECK_LIB(mzscheme,scheme_basic_env,[
      AC_MSG_RESULT(yes)

      if test -n "$mzscheme_loc"; then
        AC_MSG_WARN([MzScheme is not nice, please check that $mzscheme_loc/lib is in /etc/ld.so.conf or TPServer-cpp WILL NOT RUN])
      fi

      AM_CONDITIONAL(MZSCHEME, true)
      mzscheme="yes"
      MODULE_LIBS="$MODULE_LIBS -L../modules/tpcl/mzscheme -ltpmzscheme"
   ],[
      AM_CONDITIONAL(MZSCHEME, false)
   ])
else
  AM_CONDITIONAL(MZSCHEME, false)
fi

if test "$guile_option" != "no"; then
   AC_PATH_PROG(GUILE_CONFIG,guile-config,no,$guile_option:$PATH)
   if test "$GUILE_CONFIG" != "no"; then
      CPPFLAGS="$CPPFLAGS `$GUILE_CONFIG compile`"
      LDFLAGS="$LDFLAGS `$GUILE_CONFIG link`"
      AC_DEFINE(HAVE_GUILE, 1, [Defined to 1 if guile libraries are available])
      AM_CONDITIONAL(GUILE, true)
      guile_use="yes"
      MODULE_LIBS="$MODULE_LIBS -L../modules/tpcl/guile -ltpguile"
   else
      AC_MSG_WARN(Cannot find Guile configuration script, not building Guile module)
      AM_CONDITIONAL(GUILE, false)
   fi

else
  AM_CONDITIONAL(GUILE, false)
fi

# We need a scheme interpreter.  Require either MzScheme or Guile.
if test "$mzscheme_use" != "yes" -a "$guile_use" != "yes"; then
   AC_MSG_ERROR([Either MzScheme or Guile must be used.  Please install mzscheme or guile. See ./configure --help if one is already installed.])
fi


# ===================================================
# MySQL tests
# ===================================================
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
        MYSQL_LIBS=`${MYSQL_CONFIG} --libs_r`
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
	MODULE_LIBS="$MODULE_LIBS -L../modules/persistence/mysql -ltpmysql"
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

AC_SUBST(MODULE_LIBS)

AC_CONFIG_FILES([
Makefile
tpserver/Makefile
modules/Makefile
modules/games/Makefile
modules/games/minisec/Makefile
modules/persistence/Makefile
modules/persistence/mysql/Makefile
modules/tpcl/Makefile
modules/tpcl/guile/Makefile
modules/tpcl/mzscheme/Makefile
]) 

AC_OUTPUT
