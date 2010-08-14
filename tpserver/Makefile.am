## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tpserver-cpp

if GNUTLS
GNUTLSSOURCES = tlsmanager.cpp tlssocket.cpp \
		playertlsconn.cpp httpssocket.cpp tlsexception.cpp
GNUTLSHEADERS = tlsmanager.h tlssocket.h playertlsconn.h httpssocket.h tlsexception.h
else
GNUTLSSOURCES =
GNUTLSHEADERS =
endif

if AVAHI
AVAHISOURCES = avahi.cpp
AVAHIHEADERS = avahi.h
else
AVAHISOURCES =
AVAHIHEADERS =
endif

tpserver_cpp_SOURCES = $(GNUTLSSOURCES) main.cpp protocolobject.cpp connection.cpp tcpconnection.cpp \
		frame.cpp inputframe.cpp outputframe.cpp game.cpp logging.cpp net.cpp settings.cpp player.cpp object.cpp \
		order.cpp systemexception.cpp frameexception.cpp \
		vector3d.cpp board.cpp message.cpp ordermanager.cpp objecttypemanager.cpp \
		playerconnection.cpp tcpsocket.cpp property.cpp \
		propertyvalue.cpp component.cpp design.cpp designstore.cpp tpscheme.cpp category.cpp \
		persistence.cpp objectmanager.cpp playermanager.cpp listensocket.cpp boardmanager.cpp \
		filelogger.cpp syslogger.cpp consolelogger.cpp resourcedescription.cpp \
		resourcemanager.cpp pluginmanager.cpp httpsocket.cpp playerhttpconn.cpp \
		admintcpsocket.cpp adminconnection.cpp adminlogger.cpp \
		command.cpp commandmanager.cpp \
		$(AVAHISOURCES) advertiser.cpp metaserverpublisher.cpp metaserverconnection.cpp \
		orderparameter.cpp orderparameters.cpp \
		prng.cpp playeragent.cpp turnprocess.cpp \
		objectparameter.cpp objectparametergroup.cpp position3dobjectparam.cpp \
		velocity3dobjectparam.cpp sizeobjectparam.cpp integerobjectparam.cpp \
		referenceobjectparam.cpp refquantitylistobjectparam.cpp resourcelistobjectparam.cpp \
		orderqueueobjectparam.cpp orderqueue.cpp turntimer.cpp asynctimeremaining.cpp \
		playerview.cpp componentview.cpp designview.cpp objectview.cpp objecttype.cpp \
		objectparametergroupdesc.cpp objectbehaviour.cpp \
		basicturntimer.cpp thresholdturntimer.cpp playersfinishedturntimer.cpp \
		mediaobjectparam.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp \
		tinyxml/tinyxmlparser.cpp battlexml/combatant.cpp battlexml/battleexceptions.cpp \
		battlexml/battlelogger.cpp rulesetsupport/nameselector.cpp
		
headersdir = $(includedir)/tpserver
headers_HEADERS = modifiable.h common.h algorithms.h connection.h tcpconnection.h packable.h protocolobject.h protocolview.h \
	identifiable.h nameable.h describable.h manager.h \
	frame.h inputframe.h outputframe.h game.h logging.h net.h settings.h \
	player.h object.h \
	order.h systemexception.h frameexception.h \
	vector3d.h board.h message.h ordermanager.h \
	objecttypemanager.h \
	playerconnection.h adminconnection.h \
	refsys.h property.h propertyvalue.h component.h \
	design.h designstore.h \
	ruleset.h tpscheme.h category.h persistence.h \
	objectmanager.h \
	playermanager.h listensocket.h boardmanager.h \
	resourcedescription.h resourcemanager.h  \
	orderparameter.h orderparameters.h  \
	timercallback.h prng.h playeragent.h turnprocess.h \
	objectparameter.h objectparametergroup.h position3dobjectparam.h \
	velocity3dobjectparam.h sizeobjectparam.h integerobjectparam.h \
	referenceobjectparam.h refquantitylistobjectparam.h resourcelistobjectparam.h \
	orderqueueobjectparam.h orderqueue.h turntimer.h playerview.h componentview.h \
	designview.h objectview.h objecttype.h objectparametergroupdesc.h \
	objectbehaviour.h protocol.h \
	command.h commandmanager.h mediaobjectparam.h

headerstinyxmldir = $(headersdir)/tinyxml
headerstinyxml_HEADERS = tinyxml/tinyxml.h

headersbattlexmldir = $(headersdir)/battlexml
headersbattlexml_HEADERS = battlexml/combatant.h battlexml/battleexceptions.h battlexml/battlelogger.h

headersrulesetsupportdir = $(headersdir)/rulesetsupport
headersrulesetsupport_HEADERS = rulesetsupport/nameselector.h

noinst_HEADERS = tcpsocket.h filelogger.h syslogger.h \
	consolelogger.h logsink.h $(GNUTLSHEADERS) pluginmanager.h httpsocket.h \
	playerhttpconn.h $(AVAHIHEADERS) advertiser.h publisher.h metaserverpublisher.h \
	admintcpsocket.h adminlogger.h \
	metaserverconnection.h asyncframe.h asynctimeremaining.h basicturntimer.h \
	thresholdturntimer.h playersfinishedturntimer.h

INCLUDES = $(LTDLINCL)

# FIXME: the dlopens should be dependent on what is configured to run.
tpserver_cpp_LDADD = @libgnutls_LIBS@ @avahiclient_LIBS@ @BOOST_LDFLAGS@ @BOOST_SIGNALS_LIB@ $(LIBLTDL) "-dlopen" self
tpserver_cpp_DEPENDENCIES =
AM_CXXFLAGS = @libgnutls_CFLAGS@ @avahiclient_CFLAGS@
tpserver_cpp_LDFLAGS = -export-dynamic 


