#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "logging.h"
#include "net.h"
#include "frame.h"
#include "game.h"
#include "player.h"

#include "connection.h"

Connection::Connection()
{
	status = 0;
	player = NULL;
}

Connection::Connection(Connection & rhs)
{
	sockfd = rhs.sockfd;
	status = rhs.status;
	player = rhs.player;
}

Connection::Connection(int fd)
{
	sockfd = fd;
	status = 1;
	Network::getNetwork()->addFD(fd);
	player = NULL;
}

Connection::~Connection()
{
	if (status != 0) {
		close();
	}
}

Connection Connection::operator=(Connection & rhs)
{
	sockfd = rhs.sockfd;
	status = rhs.status;
	player = rhs.player;
	return *this;
}

int Connection::getFD()
{
	return sockfd;
}

void Connection::setFD(int fd)
{
	sockfd = fd;
	status = 1;
	Network::getNetwork()->addFD(fd);
}

void Connection::process()
{
	Logger::getLogger()->debug("About to Process");
	switch (status) {
	case 1:
		//check if user is really a TP protocol ver1 client
		verCheck();
		break;
	case 2:
		//authorise the user
		login();
		break;
	case 3:
		//process as normal
		inGameFrame();
		break;
	case 0:
	default:
		//do nothing
		Logger::getLogger()->warning("Tryed to process connections that is closed or invalid");
		if (status != 0)
			close();
		status = 0;
		break;
	}
	Logger::getLogger()->debug("Finished Processing");
}

void Connection::close()
{
	Logger::getLogger()->debug("Closing connection");
	Network::getNetwork()->removeFD(sockfd);
	if (player != NULL) {
		player->setConnection(NULL);
		player = NULL;
	}
	::close(sockfd);
	status = 0;
}

void Connection::sendFrame(Frame * frame)
{
	char *packet = frame->getPacket();
	if (packet != NULL) {
		int len = frame->getLength() + 12;
		send(sockfd, packet, len, 0);
		delete[]packet;
	} else {
		Logger::getLogger()->warning("Could not get packet from frame to send");
	}
	delete frame;
}

Frame* Connection::createFrame(Frame* oldframe)
{
  Frame* newframe = new Frame(version);
  if(oldframe != NULL) {
    newframe->setSequence(oldframe->getSequence());
  } else {
    newframe->setSequence(0);
  }
  return newframe;
}

int Connection::getStatus()
{
	return status;
}

void Connection::verCheck()
{
	char *buff = new char[4];
	int len = read(sockfd, buff, 4);
	if (len == 4 && memcmp(buff, "TP01", 4) == 0) {
	  // check the rest of the packet
	  delete[] buff;
	  buff = new char[8];
	  len = read(sockfd, buff, 8);
	  if(len == 6 && memcmp(buff, "\0\0\0\0\0\0\0\0", 8) == 0) {
		status = 2;
		Logger::getLogger()->info("Client has correct version of protocol");
		version = fv0_1;
		Frame *okframe = createFrame();
		okframe->setType(ft_OK);
		okframe->packString("Protocol check ok, continue");
		sendFrame(okframe);
	  } else {

	  }
	} else if (len == 4 && memcmp(buff, "TPO2", 4) == 0) {
	  // check the rest of the packet
	  delete[] buff;
	  buff = new char[12];
	  len = read(sockfd, buff, 12);
	  if (len == 12){
	    int seqNum = 0;

	    status = 2;
	    Logger::getLogger()->info("Client has version 2 of protocol");
	    version = fv0_2;
	    Frame *okframe = createFrame(NULL);
	    // since we don't create a frame object, we have to set the sequence number seperately
	    okframe->setSequence(seqNum);
	    okframe->setType(ft02_OK);
	    okframe->packString("Protocol check ok, contunue");
	    sendFrame(okframe);
	  } else {

	  }
	} else {
		Logger::getLogger()->warning("Client did not show correct version of protocol");
		// send "I don't understand" message
		if (len != 0) {
			Frame *failframe = new Frame();
			failframe->createFailFrame(0, "You are not running the correct protocol");	// TODO - should be a const or enum, protocol error
			sendFrame(failframe);
		}
		close();
	}

	delete[]buff;
}

void Connection::login()
{
	Frame *recvframe = createFrame();
	if (readFrame(recvframe)) {
		char *username = recvframe->unpackString();
		char *password = recvframe->unpackString();
		if (username != NULL && password != NULL) {
			//authenicate
			player = Game::getGame()->findPlayer(username, password);
			if (player != NULL) {
				Frame *okframe = createFrame(recvframe);
				okframe->setType(ft_OK);
				okframe->packString("Welcome");
				sendFrame(okframe);
				player->setConnection(this);
				status = 3;
			} else {
				Logger::getLogger()->debug("bad username or password");
				Frame *failframe = createFrame(recvframe);
				failframe->createFailFrame(1, "Login Error - bad username or password");	// TODO - should be a const or enum, Login error
				sendFrame(failframe);
			}
		} else {
			Logger::getLogger()->debug("username or password == NULL");
			Frame *failframe = createFrame(recvframe);
			failframe->createFailFrame(1, "Login Error - no username or password");	// TODO - should be a const or enum, Login error
			sendFrame(failframe);
			close();
		}
		if (username != NULL)
			delete username;
		if (password != NULL)
			delete password;

	}

	delete recvframe;

}

void Connection::inGameFrame()
{
	Frame *frame = createFrame();
	if (readFrame(frame)) {
		//Logger::getLogger()->warning("Discarded frame, not processed");
		//todo
		// should pass frame to player to do something with
		player->processIGFrame(frame);
	} else {
		// client closed
		delete frame;
	}
}

bool Connection::readFrame(Frame * recvframe)
{
	bool rtn;
	char *headerbuff = new char[12];
	int len = read(sockfd, headerbuff, 12);
	if (len == 12) {
		if ((len = recvframe->setHeader(headerbuff)) != -1) {
			char *data = new char[len];
			int dlen = read(sockfd, data, len);
			if (len != dlen) {
				//have to think about this.... what do we do?
				Logger::getLogger()->debug("Read data not the length needed");
			}
			recvframe->setData(data, dlen);
			delete[]data;
			rtn = true;
		} else {
			Logger::getLogger()->debug("Incorrect header");
			// protocol error
			Frame *failframe = createFrame();
			failframe->createFailFrame(0, "Protocol Error");	// TODO - should be a const or enum, protocol error
			sendFrame(failframe);
			close();
			rtn = false;
		}
	} else {
		Logger::getLogger()->debug("Did not read 12 bytes");
		if (len > 0) {
			Frame *failframe = createFrame();
			failframe->createFailFrame(0, "Protocol Error");	// TODO - should be a const or enum, protocol error
			sendFrame(failframe);
		} else {
			Logger::getLogger()->info("Client disconnected");
		}
		close();
		rtn = false;
	}
	delete[]headerbuff;
	return rtn;
}
