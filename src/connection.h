#ifndef CONNECTION_H
#define CONNECTION_H

class Frame;
class Player;

// this header should be removed
#include "frame.h"

class Connection {

      public:

	Connection();
	Connection(Connection & rhs);
	Connection(int fd);
	~Connection();
	Connection operator=(Connection & rhs);

	int getFD();
	void setFD(int fd);

	void process();
	void close();
	void sendFrame(Frame * frame);

	Frame* createFrame(Frame* oldframe = NULL);

	int getStatus();

	FrameVersion getProtocolVersion();

      private:

	void verCheck();
	void login();

	void inGameFrame();

	bool readFrame(Frame * recvframe);

	int sockfd;
	Player *player;
	int status;

	FrameVersion version;

};

#endif
