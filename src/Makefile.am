## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tpserver-cpp
EXTRA_PROGRAMS = guile_test

COMMONSOURCES = main.cpp connection.cpp console.cpp \
		frame.cpp game.cpp logging.cpp net.cpp \
		settings.cpp connection.h console.h \
		frame.h game.h logging.h net.h settings.h \
		player.cpp player.h object.cpp object.h \
		order.cpp order.h objectdata.h objectdata.cpp \
		universe.h universe.cpp ownedobject.h \
		ownedobject.cpp emptyobject.h emptyobject.cpp \
		planet.h planet.cpp fleet.h fleet.cpp nop.h \
		nop.cpp move.h move.cpp build.h build.cpp \
		vector3d.h vector3d.cpp board.h board.cpp \
		message.h message.cpp colonise.h colonise.cpp \
		splitfleet.h splitfleet.cpp mergefleet.h \
		mergefleet.cpp ordermanager.h ordermanager.cpp \
		objectdatamanager.h objectdatamanager.cpp \
		combatstrategy.h combatstrategy.cpp rspcombat.h \
		rspcombat.cpp playerconnection.h playerconnection.cpp \
		playertcpconn.h playertcpconn.cpp tcpsocket.h \
		tcpsocket.cpp refsys.h property.h property.cpp \
		propertyvalue.h propertyvalue.cpp component.h \
		component.cpp design.h design.cpp designstore.h \
		designstore.cpp ruleset.h minisec.h minisec.cpp \
		tpscheme.h tpscheme.cpp category.h category.cpp \
		persistence.h persistence.cpp objectmanager.h \
		objectmanager.cpp playermanager.h playermanager.cpp \
		listensocket.h listensocket.cpp boardmanager.h \
		boardmanager.cpp filelogger.h filelogger.cpp \
		syslogger.h syslogger.cpp consolelogger.h \
		consolelogger.cpp logsink.h


MZSCHEMESOURCES = tpmzscheme.h tpmzscheme.cpp
GUILESOURCES = tpguile.h tpguile.cpp

if GUILE
SCHEMESOURCES = $(GUILESOURCES)
else
SCHEMESOURCES = $(MZSCHEMESOURCES)
endif

if MYSQL
DBSOURCES = mysqlpersistence.h mysqlpersistence.cpp \
		mysqlobjecttype.h mysqlobjecttype.cpp \
		mysqluniverse.h mysqluniverse.cpp \
		mysqlemptyobject.h mysqlemptyobject.cpp \
		mysqlplanet.h mysqlplanet.cpp \
		mysqlfleet.h mysqlfleet.cpp \
		mysqlordertype.h mysqlordertype.cpp \
		mysqlordernop.h mysqlordernop.cpp \
		mysqlorderbuild.h mysqlorderbuild.cpp \
		mysqlordercolonise.h mysqlordercolonise.cpp \
		mysqlordermove.h mysqlordermove.cpp \
		mysqlordermergefleet.h mysqlordermergefleet.cpp \
		mysqlordersplitfleet.h mysqlordersplitfleet.cpp
else
DBSOURCES =
endif

if GNUTLS
GNUTLSSOURCES = tlsmanager.h tlsmanager.cpp tlssocket.h tlssocket.cpp \
		playertlsconn.h playertlsconn.cpp httpssocket.h httpssocket.cpp
else
GNUTLSSOURCES =
endif

tpserver_cpp_SOURCES = $(COMMONSOURCES) $(SCHEMESOURCES) $(DBSOURCES) $(GNUTLSSOURCES)

tpserver_cpp_LDADD = $(READLINE_LIBS) $(MYSQL_LIBS) $(LIBGNUTLS_LIBS)

guile_test_SOURCES = test/guile_test.cpp tpscheme.h tpscheme.cpp game.h game.cpp \
	consolelogger.h consolelogger.cpp filelogger.h filelogger.cpp logging.h \
	logging.cpp logsink.h syslogger.h syslogger.cpp property.h property.cpp \
	propertyvalue.h propertyvalue.cpp design.h design.cpp designstore.h \
	designstore.cpp player.h player.cpp playerconnection.h playerconnection.cpp \
	frame.h frame.cpp connection.h connection.cpp playermanager.h \
	playermanager.cpp message.h message.cpp settings.h settings.cpp component.h \
	component.cpp board.h board.cpp boardmanager.h boardmanager.cpp \
	category.h category.cpp objectmanager.h objectmanager.cpp object.h \
	object.cpp vector3d.h vector3d.cpp objectdata.h objectdata.cpp \
	objectdatamanager.h objectdatamanager.cpp ordermanager.h ordermanager.cpp \
	order.h order.cpp ownedobject.h ownedobject.cpp combatstrategy.h \
	combatstrategy.cpp net.h net.cpp universe.h universe.cpp listensocket.h \
	listensocket.cpp tcpsocket.h tcpsocket.cpp playertcpconn.h \
	playertcpconn.cpp $(SCHEMESOURCES) $(GNUTLSSOURCES)

guile_test_LDADD = $(LIBGNUTLS_LIBS)

data_DATA = designstruct.scm
EXTRA_DIST = designstruct.scm
