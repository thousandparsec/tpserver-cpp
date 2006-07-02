## Process this file with automake to produce Makefile.in

persistencelibdir = $(libdir)/tpserver/persistence

if MYSQL
persistencelib_LTLIBRARIES = libtpmysql.la
else
persistencelib_LTLIBRARIES =
endif

libtpmysql_la_SOURCES = mysqlpersistence.h mysqlpersistence.cpp \
		mysqlobjecttype.h mysqlobjecttype.cpp \
		mysqluniverse.h mysqluniverse.cpp \
		mysqlordertype.h mysqlordertype.cpp \
		mysqlordernop.h mysqlordernop.cpp

libtpmysql_la_LIBADD = $(MYSQL_LIBS)
libtpmysql_la_LDFLAGS = -module

EXTRA_DIST = sample.conf
