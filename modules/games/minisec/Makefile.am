## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libminisec.la

rulesetlibdir = $(libdir)/tpserver/ruleset

libminisec_la_SOURCES = universe.cpp planet.cpp fleet.cpp \
		nop.cpp move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		rspcombat.cpp minisec.cpp \
		minisecturn.cpp emptyobject.cpp \
		combatant.cpp spaceobject.cpp ownedobject.cpp \
        intercept.cpp
		
libminisec_la_LDFLAGS = -module

noinst_HEADERS = planet.h fleet.h nop.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		universe.h emptyobject.h rspcombat.h minisec.h minisecturn.h \
		combatant.h spaceobject.h ownedobject.h intercept.h

EXTRA_DIST = sample.conf tpserver-cpp-minisec.xml

tpconfigdir = $(datadir)/tp

tpconfig_DATA = tpserver-cpp-minisec.xml
