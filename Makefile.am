SUBDIRS = libltdl modules tpserver

man_MANS = tpserver-cpp.1

EXTRA_DIST = $(man_MANS) sample.conf quickstart-minisec.conf quickstart-mtsec.conf autogen.sh \
	quickstart-rfts.conf quickstart-tae.conf quickstart-risk-bid.conf quickstart-risk-random.conf \
	risk-defaultmap.svg \
	m4/ax_boost_base.m4 m4/ax_boost_signals.m4 m4/ax_libltdl.m4 tpserver-cpp.xml.in

tpconfigdir = $(datadir)/tp/servers

tpconfig_DATA = tpserver-cpp.xml
