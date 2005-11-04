bin_PROGRAMS = tpserver-cpp
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
		tpscheme.h tpscheme.cpp category.h category.cpp
if MZSCHEME
tpserver_cpp_SOURCES = $(COMMONSOURCES) tpmzscheme.h tpmzscheme.cpp
else
tpserver_cpp_SOURCES = $(COMMONSOURCES)
endif

tpserver_cpp_LDADD = $(READLINE_LIBS)

data_DATA = designstruct.scm

