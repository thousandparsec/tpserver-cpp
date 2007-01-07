## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libmtsec.la

if MYSQL
MYSQL_MTSEC_SOURCE = mysqlemptyobject.h mysqlemptyobject.cpp \
	mysqlfleet.h mysqlfleet.cpp \
	mysqlplanet.h mysqlplanet.cpp
else
MYSQL_MTSEC_SOURCE =
endif

rulesetlibdir = $(libdir)/tpserver/ruleset

libmtsec_la_SOURCES = planet.cpp fleet.cpp \
		move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		avacombat.cpp mtsec.cpp \
		${MYSQL_MTSEC_SOURCE}

libmtsec_la_LDFLAGS = -module

noinst_HEADERS = planet.h fleet.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		avacombat.h mtsec.h

