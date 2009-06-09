## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libmtsec.la

rulesetlibdir = $(libdir)/tpserver/ruleset

libmtsec_la_SOURCES = universe.cpp planet.cpp fleet.cpp \
		nop.cpp move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		avacombat.cpp mtsec.cpp \
		mtsecturn.cpp emptyobject.cpp \
		spaceobject.cpp ownedobject.cpp \
		tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp \
		compsimport.cpp

libmtsec_la_LDFLAGS = -module

noinst_HEADERS = planet.h fleet.h nop.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		universe.h avacombat.h mtsec.h mtsecturn.h emptyobject.h \
		spaceobject.h ownedobject.h \
		tinystr.h tinyxml.h compsimport.h

EXTRA_DIST = tpserver-cpp-mtsec.xml

tpconfigdir = $(datadir)/tp
tpconfig_DATA = tpserver-cpp-mtsec.xml
