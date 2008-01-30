## Process this file with automake to produce Makefile.in

persistencelibdir = $(libdir)/tpserver/persistence

if MYSQL
persistencelib_LTLIBRARIES = libtpmysql.la
else
persistencelib_LTLIBRARIES =
endif

libtpmysql_la_SOURCES = mysqlpersistence.h mysqlpersistence.cpp

libtpmysql_la_LIBADD = @MYSQL_LIBS@
libtpmysql_la_LDFLAGS = -export-dynamic -module
AM_CXXFLAGS = @MYSQL_CFLAGS@

EXTRA_DIST = sample.conf
