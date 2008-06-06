AC_INIT([tpserver-cpp],[0.5.0],[lee@thousandparsec.net])
AC_CONFIG_SRCDIR(tpserver/main.cpp)

AC_CANONICAL_TARGET([])

AM_INIT_AUTOMAKE(tpserver-cpp, 0.5.0)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

CXXFLAGS="-g -Wall"


AC_PROG_CC
AC_PROG_CXX
AC_LANG_CPLUSPLUS

#test if the C++ compiler actually works
AC_MSG_CHECKING([for working C++ compiler])
AC_COMPILE_IFELSE([
#include <iostream>
using namespace std;
#ifdef __cplusplus
int main(int n, char** c){
  cout << "This is a working c++ compiler" << endl;
  return 0;
}
#else
#error Not a c++ compiler
#endif

],AC_MSG_RESULT(yes), 
AC_MSG_ERROR(C++ compiler not working)
)

AC_REQUIRE_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl Enable building of the convenience library
dnl and set LIBLTDL accordingly
AC_LIBLTDL_CONVENIENCE
dnl Substitute LTDLINCL and LIBLTDL in the Makefiles
AC_SUBST(LTDLINCL)
AC_SUBST(LIBLTDL)
dnl Check for dlopen support
AC_LIBTOOL_DLOPEN
dnl Configure libtool
AC_PROG_LIBTOOL
dnl Configure libltdl
AC_CONFIG_SUBDIRS(libltdl)


AC_HEADER_STDC



AC_CHECK_LIB(pthread, pthread_create,,AC_MSG_ERROR(TP Server requires pthread library))
#check headers, functions, etc

AC_CHECK_LIB(dl, dlsym,,)

PKG_CHECK_MODULES(avahiclient, avahi-client > 0.6.0,
    [
    AC_DEFINE(HAVE_AVAHI, 1, true if avahi is available)
    AM_CONDITIONAL(AVAHI, true)
    ],[
    AM_CONDITIONAL(AVAHI, false)
    ]
)
AC_SUBST([avahiclient_LIBS])
AC_SUBST([avahiclient_CFLAGS])

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


#check for boost
# Random and smartpointer are in base
AX_BOOST_BASE([1.32.1])

# Signal tested separately
 AX_BOOST_SIGNALS

CXXFLAGS="$CXXFLAGS $BOOST_CPPFLAGS"

CXXFLAGS="$CXXFLAGS $BOOST_CPPFLAGS"

PKG_CHECK_MODULES(libgnutls, gnutls >= 1.2.10,
  [
    AC_DEFINE(HAVE_LIBGNUTLS, 1, Define to 1 if libgnutls is available)
    AM_CONDITIONAL(GNUTLS, true)
  ],
  [
    AM_CONDITIONAL(GNUTLS, false)
  ]
)
AC_SUBST([libgnutls_LIBS])
AC_SUBST([libgnutls_CFLAGS])

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
   if test -d "$mzscheme_loc" ; then
     MZSCHEME_CFLAGS="-I$mzscheme_loc/include"
     MZSCHEME_LIBS="-L$mzscheme_loc/lib"
   fi

   SAVE_CXXFLAGS=$CXXFLAGS
   SAVE_LDFLAGS=$LDFLAGS
   
   CXXFLAGS="$CXXFLAGS $MZSCHEME_CFLAGS"
   LDFLAGS="$LDFLAGS $MZSCHEME_LDFLAGS"

   AC_CHECK_HEADER(scheme.h, AC_DEFINE(HAVE_SCHEME_H, 1, [Define to 1 if scheme.h is present]))
   AC_CHECK_HEADER(plt/scheme.h, AC_DEFINE(HAVE_PLT_SCHEME_H, 1, [Define to 1 if plt/scheme.h is present]))
   if test -n "ac_cv_header_scheme_h" && test -n "ac_cv_header_plt_scheme_h" ; then

      AC_CHECK_LIB(mzgc, GC_init,
        [
        MZSCHEME_LIBS="$MZSCHEME_LIBS -lmzgc"
        LDFLAGS="$LDFLAGS -lmzgc"
        ]
      ,)
      AC_CHECK_LIB(mzscheme,scheme_basic_env,[
          MZSCHEME_LIBS="$MZSCHEME_LIBS -lmzscheme"
          AC_DEFINE(HAVE_MZSCHEME, 1, [Defined to 1 if MzScheme libraries are available])
    
          # Test for MZScheme 2.x vs. 3.x
          AC_MSG_CHECKING(mzscheme version)
          AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
              #ifdef HAVE_SCHEME_H
              #include "scheme.h"
              #else
              #include "plt/scheme.h"
              #endif
              #ifndef SCHEME_STRINGP
              #error
              #endif]])],
              [AC_DEFINE(HAVE_MZSCHEME20X, 1,Define if we have mzscheme 209)
              AC_MSG_RESULT(20x)],
              AC_MSG_RESULT(3xx))
    
          if test -d "$mzscheme_loc"; then
            AC_MSG_WARN([MzScheme is not nice, please check that $mzscheme_loc/lib is in /etc/ld.so.conf or TPServer-cpp WILL NOT RUN])
          fi
    
          AM_CONDITIONAL(MZSCHEME, true)
          mzscheme_use="yes"
      ],[
          AC_CHECK_LIB(mzscheme3m, scheme_basic_env, [
                MZSCHEME_LIBS="$MZSCHEME_LIBS -lmzscheme3m"
                AC_DEFINE(HAVE_MZSCHEME, 1, [Defined to 1 if MzScheme libraries are available])
                AM_CONDITIONAL(MZSCHEME, true)
                mzscheme_use="yes"
            ],[
                AC_MSG_WARN(Not going to build MzScheme module)
                AM_CONDITIONAL(MZSCHEME, false)
            ])
      ])
    else
      AM_CONDITIONAL(MZSCHEME, false)
    fi
    CXXFLAGS=$SAVE_CXXFLAGS
    LDFLAGS=$SAVE_LDFLAGS
else
  AM_CONDITIONAL(MZSCHEME, false)
fi
AC_SUBST([MZSCHEME_CFLAGS])
AC_SUBST([MZSCHEME_LIBS])


if test "$guile_option" != "no"; then
   AC_PATH_PROG(GUILE_CONFIG,guile-config,no,$guile_option:/sw/share/guile/1.8/scripts/binoverride:$PATH)
   if test "$GUILE_CONFIG" != "no"; then
      dnl autoconf Guile version test adapted from SWIG test in xapian-bindings
      v=`$GUILE_CONFIG --version 2>&1|sed 's/^guile-config - Guile version \([[0-9\.]]*\).*/\1/p;d'`
      case $v in
      [1.6.*])
        GUILE_CFLAGS="`$GUILE_CONFIG compile`"
        GUILE_LIBS="`$GUILE_CONFIG link`"
        AC_DEFINE(HAVE_GUILE, 1, [Defined to 1 if guile libraries are available])
        AC_DEFINE(HAVE_GUILE1_6, 1, [Defined to 1 if the guile version is 1.6])
        AM_CONDITIONAL(GUILE, true)
        guile_use="yes"
        ;;
      [1.[789].*|2.*]) 
        GUILE_CFLAGS="`$GUILE_CONFIG compile`"
        GUILE_LIBS="`$GUILE_CONFIG link`"
        AC_DEFINE(HAVE_GUILE, 1, [Defined to 1 if guile libraries are available])
        AM_CONDITIONAL(GUILE, true)
        guile_use="yes"
        ;;
      *)
        AC_MSG_WARN([guile >= 1.6.* required (you have $v)]) 
        AM_CONDITIONAL(GUILE, false)
        ;;
      esac
   else
      AC_MSG_WARN(Cannot find Guile configuration script, not building Guile module)
      AM_CONDITIONAL(GUILE, false)
   fi

else
  AM_CONDITIONAL(GUILE, false)
fi

AC_SUBST([GUILE_CFLAGS])
AC_SUBST([GUILE_LIBS])

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
        AC_MSG_RESULT($MYSQL_CFLAGS)
        no_mysql=no
    fi
    
    if test "$no_mysql" = "yes"; then
        AM_CONDITIONAL(MYSQL, false)
    else
        AC_DEFINE(HAVE_LIBMYSQL, 1, [Define to 1 if you have the `mysql' library (-lmysql).])
        AM_CONDITIONAL(MYSQL, true)
    fi
fi

AC_SUBST([MYSQL_CFLAGS])
AC_SUBST([MYSQL_LIBS])


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
AC_DEFINE_DIR(LIBDIR, libdir, The library directory where the modules are stored)

#AC_SUBST(MODULE_LIBS)

AC_CONFIG_FILES([
Makefile
tpserver/Makefile
modules/Makefile
modules/games/Makefile
modules/games/minisec/Makefile
modules/games/mtsec/Makefile
modules/games/rfts/Makefile
modules/persistence/Makefile
modules/persistence/mysql/Makefile
modules/tpcl/Makefile
modules/tpcl/guile/Makefile
modules/tpcl/mzscheme/Makefile
]) 

AC_OUTPUT
