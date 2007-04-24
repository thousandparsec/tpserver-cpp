## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libminisec.la

rulesetlibdir = $(libdir)/tpserver/ruleset

libminisec_la_SOURCES = planet.cpp fleet.cpp \
		move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		rspcombat.cpp minisec.cpp \
		minisecturn.cpp
		
libminisec_la_LDFLAGS = -module

noinst_HEADERS = planet.h fleet.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		rspcombat.h minisec.h minisecturn.h

