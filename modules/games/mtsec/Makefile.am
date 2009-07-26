## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libmtsec.la

rulesetlibdir = $(libdir)/tpserver/ruleset

libmtsec_la_SOURCES = universe.cpp planet.cpp fleet.cpp \
		nop.cpp move.cpp buildfleet.cpp sendpoints.cpp \
		colonise.cpp enhance.cpp \
		splitfleet.cpp mergefleet.cpp \
		avacombat.cpp mtsec.cpp \
		mtsecturn.cpp emptyobject.cpp \
		spaceobject.cpp ownedobject.cpp \
		xmlimport.cpp

libmtsec_la_LDFLAGS = -module

noinst_HEADERS = planet.h fleet.h nop.h move.h buildfleet.h sendpoints.h colonise.h splitfleet.h mergefleet.h  \
		enhance.h universe.h avacombat.h mtsec.h mtsecturn.h emptyobject.h \
		spaceobject.h ownedobject.h 

EXTRA_DIST = tpserver-cpp-mtsec.xml

tpconfigdir = $(datadir)/tp
tpconfig_DATA = tpserver-cpp-mtsec.xml
