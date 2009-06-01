## Process this file with automake to produce Makefile.in

rulesetlib_LTLIBRARIES = librisk.la

rulesetlibdir = $(libdir)/tpserver/ruleset

#All .cpp's
librisk_la_SOURCES = risk.cpp colonize.cpp move.cpp reinforce.cpp riskturn.cpp map.cpp staticobject.cpp universe.cpp ownedobject.cpp planet.cpp wormhole.cpp constellation.cpp resourcelistparam.cpp graph.cpp mapimport.cpp


librisk_la_LDFLAGS = -module

#All the .h's
noinst_HEADERS = risk.h colonize.h move.h reinforce.h riskturn.h map.h staticobject.h containertypes.h universe.h ownedobject.h planet.h wormhole.h constellation.h resourcelistparam.h graph.h mapimport.h

EXTRA_DIST = sample.conf tpserver-cpp-risk.xml

tpconfigdir = $(datadir)/tp
tpconfig_DATA = tpserver-cpp-risk.xml
