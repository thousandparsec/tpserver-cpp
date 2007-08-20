## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = librfts.la

rulesetlibdir = $(libdir)/tpserver/ruleset

librfts_la_SOURCES = bombard.cpp buildfleet.cpp colonise.cpp fleet.cpp map.cpp \
	mergefleet.cpp move.cpp ownedobject.cpp planet.cpp playerinfo.cpp productioninfo.cpp \
	productionorder.cpp renamefleet.cpp resourcelistparam.cpp rfts.cpp rftsturn.cpp \
	splitfleet.cpp staticobject.cpp universe.cpp

librfts_la_LDFLAGS = -module

noinst_HEADERS = bombard.h buildfleet.h colonise.h containertypes.h fleet.h \
	map.h mergefleet.h move.h ownedobject.h planet.h playerinfo.h \
	productioninfo.h productionorder.h renamefleet.h resourcelistparam.h rfts.h rftsturn.h \
	splitfleet.h staticobject.h universe.h
