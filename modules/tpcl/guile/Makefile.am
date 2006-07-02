## Process this file with automake to produce Makefile.in


if GUILE
tpschemelib_LTLIBRARIES = libtpguile.la
else
tpschemelib_LTLIBRARIES = 
endif

tpschemelibdir = $(libdir)/tpserver/tpscheme
libtpguile_la_SOURCES = tpguile.h tpguile.cpp
libtpguile_la_LDFLAGS = -module

