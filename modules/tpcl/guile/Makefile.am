## Process this file with automake to produce Makefile.in


if GUILE
tpschemelib_LTLIBRARIES = libtpguile.la
tpdata_DATA = guile.scm
else
tpschemelib_LTLIBRARIES = 
endif

tpschemelibdir = $(libdir)/tpserver/tpscheme
tpdatadir = $(datadir)/tpserver
libtpguile_la_SOURCES = tpguile.h tpguile.cpp
libtpguile_la_LDFLAGS = -module

EXTRA_DIST = guile.scm