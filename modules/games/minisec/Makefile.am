## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libminisec.la

if MYSQL
MYSQL_MINISEC_SOURCE = mysqlemptyobject.h mysqlemptyobject.cpp \
	mysqlfleet.h mysqlfleet.cpp \
	mysqlorderbuild.h mysqlorderbuild.cpp \
	mysqlordercolonise.h mysqlordercolonise.cpp \
	mysqlordermergefleet.h mysqlordermergefleet.cpp \
	mysqlordermove.h mysqlordermove.cpp \
	mysqlordersplitfleet.h mysqlordersplitfleet.cpp \
	mysqlplanet.h mysqlplanet.cpp
else
MYSQL_MINISEC_SOURCE = 
endif

libminisec_la_SOURCES = planet.cpp fleet.cpp \
		move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		rspcombat.cpp minisec.cpp \
		${MYSQL_MINISEC_SOURCE}

noinst_HEADERS = planet.h fleet.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		rspcombat.h minisec.h 

