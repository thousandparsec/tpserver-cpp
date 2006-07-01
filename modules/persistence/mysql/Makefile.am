## Process this file with automake to produce Makefile.in

if MYSQL
lib_LTLIBRARIES = libtpmysql.la
libtpmysql_la_SOURCES = mysqlpersistence.h mysqlpersistence.cpp \
		mysqlobjecttype.h mysqlobjecttype.cpp \
		mysqluniverse.h mysqluniverse.cpp \
		mysqlemptyobject.h mysqlemptyobject.cpp \
		mysqlplanet.h mysqlplanet.cpp \
		mysqlfleet.h mysqlfleet.cpp \
		mysqlordertype.h mysqlordertype.cpp \
		mysqlordernop.h mysqlordernop.cpp \
		mysqlorderbuild.h mysqlorderbuild.cpp \
		mysqlordercolonise.h mysqlordercolonise.cpp \
		mysqlordermove.h mysqlordermove.cpp \
		mysqlordermergefleet.h mysqlordermergefleet.cpp \
		mysqlordersplitfleet.h mysqlordersplitfleet.cpp

libtpmysql_la_LIBADD = $(MYSQL_LIBS)

else
lib_LTLIBRARIES =
endif

EXTRA_DIST = sample.conf
