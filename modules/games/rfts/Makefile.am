## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = librfts.la

rulesetlibdir = $(libdir)/tpserver/ruleset

librfts_la_SOURCES = buildfleet.cpp staticobject.cpp fleet.cpp nop.cpp \
	planet.cpp rfts.cpp rftsturn.cpp universe.cpp

librfts_la_LDFLAGS = -module

noinst_HEADERS = buildfleet.h containertypes.h fleet.h nop.h ownedobject.h \
	planet.h rfts.h rftsturn.h staticobject.h universe.h
