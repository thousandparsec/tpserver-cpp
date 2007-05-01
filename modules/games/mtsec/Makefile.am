## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = libmtsec.la

rulesetlibdir = $(libdir)/tpserver/ruleset

libmtsec_la_SOURCES = planet.cpp fleet.cpp \
		nop.cpp move.cpp build.cpp \
		colonise.cpp \
		splitfleet.cpp mergefleet.cpp \
		avacombat.cpp mtsec.cpp \
		mtsecturn.cpp

libmtsec_la_LDFLAGS = -module

noinst_HEADERS = planet.h fleet.h nop.h move.h build.h colonise.h splitfleet.h mergefleet.h \
		avacombat.h mtsec.h mtsecturn.h

