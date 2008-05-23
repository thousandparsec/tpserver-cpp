## Process this file with automake to produce Makefile.in

#
rulesetlib_LTLIBRARIES = librisk.la

rulesetlibdir = $(libdir)/tpserver/ruleset

#All .cpp's
librisk_la_SOURCES = risk.cpp colonize.cpp move.cpp reinforce.cpp riskturn.cpp


librisk_la_LDFLAGS = -module

#All the .h's
noinst_HEADERS = risk.h colonize.h move.h reinforce.h riskturn.h

#minisec has this in it
#EXTRA_DIST = sample.conf