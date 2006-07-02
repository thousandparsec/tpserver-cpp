## Process this file with automake to produce Makefile.in

if MYSQL
lib_LTLIBRARIES = libtpmysql.la
libtpmysql_la_SOURCES = mysqlpersistence.h mysqlpersistence.cpp \
		mysqlobjecttype.h mysqlobjecttype.cpp \
		mysqluniverse.h mysqluniverse.cpp \
		mysqlordertype.h mysqlordertype.cpp \
		mysqlordernop.h mysqlordernop.cpp

libtpmysql_la_LIBADD = $(MYSQL_LIBS)

else
lib_LTLIBRARIES =
endif

EXTRA_DIST = sample.conf
