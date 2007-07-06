## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = librfts.la

rulesetlibdir = $(libdir)/tpserver/ruleset

librfts_la_SOURCES = emptyobject.cpp fleet.cpp nop.cpp planet.cpp rfts.cpp \
	rftsturn.cpp universe.cpp

librfts_la_LDFLAGS = -module

noinst_HEADERS = containertypes.h emptyobject.h fleet.h nop.h planet.h rfts.h \
	rftsturn.h universe.h
