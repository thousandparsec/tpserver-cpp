## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = librfts.la

rulesetlibdir = $(libdir)/tpserver/ruleset

librfts_la_SOURCES = buildfleet.cpp fleet.cpp move.cpp nop.cpp ownedobject.cpp \
	planet.cpp productioninfo.cpp productionorder.cpp renamefleet.cpp \
	resourcelistparam.cpp rfts.cpp rftsturn.cpp splitfleet.cpp staticobject.cpp universe.cpp

librfts_la_LDFLAGS = -module

noinst_HEADERS = buildfleet.h containertypes.h fleet.h move.h nop.h \
	ownedobject.h planet.h productioninfo.h productionorder.h renamefleet.h \
	resourcelistparam.h rfts.h rftsturn.h splitfleet.h staticobject.h universe.h
