CXXFLAGS = -g

all: tpserv

tpserv: main.o logging.o console.o game.o net.o settings.o connection.o frame.o
	g++ -o tpserv main.o logging.o console.o game.o net.o settings.o connection.o frame.o -lpthread

main.o logging.o console.o game.o net.o connection.o frame.o: logging.h

main.o console.o: console.h

main.o game.o console.o: game.h

main.o net.o console.o connection.o: net.h

main.o settings.o net.o: settings.h

connection.o net.o: connection.h

frame.o connection.o: frame.h
