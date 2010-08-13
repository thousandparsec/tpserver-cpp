SUBDIRS = libltdl modules tpserver tests
man_MANS = tpserver-cpp.1

EXTRA_DIST = $(man_MANS) sample.conf quickstart-minisec.conf quickstart-mtsec.conf autogen.sh \
	quickstart-rfts.conf quickstart-tae.conf quickstart-risk-bid.conf quickstart-risk-random.conf \
	m4/ax_boost_base.m4 m4/ax_boost_signals.m4 m4/ax_libltdl.m4 m4/shave.m4 \
	tpserver-cpp.xml.in

tpconfigdir = $(datadir)/tp

tpconfig_DATA = tpserver-cpp.xml risk-defaultmap.svg

doc: Doxyfile
	doxygen Doxyfile
