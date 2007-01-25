## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libminisec.la

if MYSQL
MYSQL_MINISEC_SOURCE = mysqlemptyobject.h mysqlemptyobject.cpp \
	mysqlfleet.h mysqlfleet.cpp \
	mysqlplanet.h mysqlplanet.cpp
else
MYSQL_MINISEC_SOURCE = 
endif

rulesetlibdir = $(libdir)/tpserver/ruleset

libminisec_la_SOURCES = planet.cpp fleet.cpp \
		move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		rspcombat.cpp minisec.cpp \
		${MYSQL_MINISEC_SOURCE}
		
libminisec_la_LDFLAGS = -module
AM_CXXFLAGS = @MYSQL_CFLAGS@

noinst_HEADERS = planet.h fleet.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		rspcombat.h minisec.h 

