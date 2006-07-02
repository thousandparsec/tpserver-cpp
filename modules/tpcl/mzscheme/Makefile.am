## Process this file with automake to produce Makefile.in


if MZSCHEME
tpschemelib_LTLIBRARIES = libtpmzscheme.la
tpdata_DATA = designstruct.scm
else
tpschemelib_LTLIBRARIES = 
endif

tpdatadir = $(datadir)/tpserver

tpschemelibdir = $(libdir)/tpserver/tpscheme
libtpmzscheme_la_SOURCES = tpmzscheme.h tpmzscheme.cpp
libtpmzscheme_la_LIBADD = -lmzscheme
libtpmzscheme_la_LDFLAGS = -module

EXTRA_DIST = designstruct.scm
