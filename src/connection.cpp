/*  Connection object, supports ipv4 and ipv6
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
		//check if user is really a TP protocol verNN client
		Logger::getLogger()->debug("Stage1 : pre-connect");
		verCheck();
		break;
	case 2:
		//authorise the user
		Logger::getLogger()->debug("Stage2 : connected");
		login();
		break;
	case 3:
		//process as normal
		Logger::getLogger()->debug("Stage3 : logged in");
		inGameFrame();
		break;
	case 0:
	default:
		//do nothing
		Logger::getLogger()->warning("Tried to process connections that is closed or invalid");
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
  if(version != frame->getVersion()){
    Logger::getLogger()->warning("Version mis-match, packet %d, connection %d", frame->getVersion(), version);
    
  }
  if(version == fv0_2 && frame->getType() >= ft02_Max){
    Logger::getLogger()->error("Tryed to send a higher than version 2 frame on a version 2 connection, not sending frame");
  }else{
	char *packet = frame->getPacket();
	if (packet != NULL) {
		int len = frame->getLength();
		send(sockfd, packet, len, 0);
		delete[] packet;
	} else {
		Logger::getLogger()->warning("Could not get packet from frame to send");
	}
	delete frame;
  }
}

Frame* Connection::createFrame(Frame* oldframe)
{
  Frame* newframe;
  if(oldframe != NULL) {
    newframe = new Frame(oldframe->getVersion());
    newframe->setSequence(oldframe->getSequence());
  } else {
    newframe = new Frame(version);
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
	// FIXME: This is bad and should be fixed!
	char *buff = new char[5];
	int len = read(sockfd, buff, 4);
	buff[4] = '\0';
	
	if (len == 4 && memcmp(buff, "TP", 2) == 0){
	  // assume we are talking TPprotocol
	  
	  int ver = atoi(buff+2);
	  if(ver == 1){
	    Logger::getLogger()->warning("Client did not show correct version of protocol (version 1)");
	    send(sockfd, "You are not running the right version of TP, please upgrade\n", 60, 0);
	    close();
	  }else if(ver > 3){
	    Logger::getLogger()->warning("Client has higher version (%d), telling it so", ver);
	    Frame *f = new Frame(fv0_3);
	    f->setSequence(0);
	    f->createFailFrame(fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");
	    sendFrame(f);
	    //stay connected just in case they try again with a lower version
	    // have to empty the receive queue though.
	    delete[] buff;
	    buff = new char[1024];
	    len = read(sockfd, buff, 1024);
	    Logger::getLogger()->debug("Read an extra %d bytes from the socket, into buffer of 1024", len);
	  }else{
	    
	    // check the rest of the packet
	    delete[] buff;
	    buff = new char[12];
	    len = read(sockfd, buff, 12);
	    
	    if (len == 12) {
	      // Sequence number
	      int seqNum = 0, nseqNum;
	      memcpy(&nseqNum, buff, 4);
	      seqNum = ntohl(nseqNum);
	      
	      // Type of packet
	      //since tp03 allows Get Feature frames before a connect frame, the first
	      // frame could be a get feature.
	      int typeNum = 0, ntypeNum;
	      memcpy(&ntypeNum, buff + 4, 4);
	      typeNum = ntohl(ntypeNum);

	      // Length of packet
	      int lenNum = 0, nlenNum;
	      memcpy(&nlenNum, buff + 8, 4);
	      lenNum = ntohl(nlenNum);
	      
	      if(ver == 3 && typeNum == ft03_Features_Get){
		version = fv0_3;
		if(lenNum != 0){
		  // ver 3 frame error, Get Features have no data.
		  Logger::getLogger()->warning("Invalid Get Features request");

		  // remove the data from the socket
		  char *cbuff = new char[lenNum];
		  len = read(sockfd, cbuff, lenNum);
		  delete[] cbuff;

		  Frame* fe = createFrame(NULL);
		  fe->setSequence(seqNum);
		  fe->createFailFrame(fec_FrameError, "GetFeatures frame should have no data");
		  sendFrame(fe);
		}else{
		  Logger::getLogger()->debug("Get Features request");
		  Frame* features = createFrame(NULL);
		  features->setSequence(seqNum);

		  Network::getNetwork()->createFeaturesFrame(features);

		  sendFrame(features);
		}
		
		delete[] buff;
		return;
	      }
	      // Read in the length of the packet and ignore
	      char *cbuff = new char[lenNum];
	      len = read(sockfd, cbuff, lenNum);

	      if(typeNum == 3){
		
		Logger::getLogger()->info("Client on connection %d is [%s]", sockfd, cbuff + 4);
		
		version = (FrameVersion)ver;

		Logger::getLogger()->info("Client has version %d of protocol", version);
		
		status = 2;
		
		Frame *okframe = createFrame(NULL);
		// since we don't create a frame object, we have to set the sequence number seperately
		okframe->setSequence(seqNum);
		okframe->setType(ft02_OK);
		
		okframe->packString("Protocol check ok, continue!");
		sendFrame(okframe);
	      }else{
		Logger::getLogger()->warning("First frame wasn't Connect or GetFeatures, was %d", typeNum);
		Frame* fe = createFrame(NULL);
		fe->setSequence(seqNum);
		fe->createFailFrame(fec_ProtocolError, "First frame wasn't Connect (or GetFeatures in tp03), please try again");
		sendFrame(fe);
	      }
	      delete[] cbuff;
	    }
	  }
	  delete[] buff;
	  return;
	  
	}
	Logger::getLogger()->warning("Client did not talk any variant of TPprotocol");
	// send "I don't understand" message
	if (len != 0) {
	  /*Frame *failframe = new Frame();
	  failframe->createFailFrame(0, "You are not running the correct protocol");	// TODO - should be a const or enum, protocol error
	  sendFrame(failframe);*/

	  send(sockfd, "You are not running the correct protocol\n", 41, 0);

	}
	close();

	delete[]buff;
}

void Connection::login()
{
	Frame *recvframe = createFrame();
	if (readFrame(recvframe)) {
	  if(recvframe->getType() == ft02_Login){
		char *username = recvframe->unpackString();
		char *password = recvframe->unpackString();
		if (username != NULL && password != NULL) {
			//authenicate
			player = Game::getGame()->findPlayer(username, password);
			if (player != NULL) {
				Frame *okframe = createFrame(recvframe);
				okframe->setType(ft02_OK);
				okframe->packString("Welcome");
				sendFrame(okframe);
				Logger::getLogger()->info("Login OK!");
				player->setConnection(this);
				status = 3;
			} else {
				Logger::getLogger()->debug("bad username or password");
				Frame *failframe = createFrame(recvframe);
				failframe->createFailFrame(fec_FrameError, "Login Error - bad username or password");	// TODO - should be a const or enum, Login error
				sendFrame(failframe);
			}
		} else {
			Logger::getLogger()->debug("username or password == NULL");
			Frame *failframe = createFrame(recvframe);
			failframe->createFailFrame(fec_FrameError, "Login Error - no username or password");	// TODO - should be a const or enum, Login error
			sendFrame(failframe);
			close();
		}
		if (username != NULL)
			delete[] username;
		if (password != NULL)
			delete[] password;

	
	  }else if(version >= fv0_3 && recvframe->getType() == ft03_Features_Get){
	    Logger::getLogger()->debug("Processing Get Features frame");
	    Frame *features = createFrame(recvframe);
	    Network::getNetwork()->createFeaturesFrame(features);
	    sendFrame(features);
	  }else{
	    Logger::getLogger()->warning("In connected state but did not receive login or get features");
	    Frame *failframe = createFrame(recvframe);
	    failframe->createFailFrame(fec_FrameError, "Wrong type of frame in this state, wanted login or get features");
	    sendFrame(failframe);
	  }
	}
	delete recvframe;
	  
}

void Connection::inGameFrame()
{
	Frame *frame = createFrame();
	if (readFrame(frame)) {
		
		if(version >= fv0_3 && frame->getType() == ft03_Features_Get){
		  Logger::getLogger()->debug("Processing Get Features frame");
		  Frame *features = createFrame(frame);
		  Network::getNetwork()->createFeaturesFrame(features);
		  sendFrame(features);
		}else{
		  // should pass frame to player to do something with
		  Logger::getLogger()->debug("inGameFrame");
		  player->processIGFrame(frame);
		}
	} else {
		Logger::getLogger()->debug("noFrame :(");
		// client closed
		delete frame;
	}
}

bool Connection::readFrame(Frame * recvframe)
{
	bool rtn;
	
	int hlen = recvframe->getHeaderLength();	
	char *headerbuff = new char[hlen];
	int len = read(sockfd, headerbuff, hlen);
	
	if (len == hlen) {
		if ( (len = recvframe->setHeader(headerbuff)) != -1 ) {
				
		  Logger::getLogger()->debug("Data Length: %d", len);
		  
				
			char *data = new char[len];
			int dlen = read(sockfd, data, len);
			
			if (len != dlen) {
				//have to think about this.... what do we do?
				Logger::getLogger()->debug("Read data not the length needed");
			}
			
			recvframe->setData(data, dlen);
			delete[] data;

			//sanity check
			if(version != recvframe->getVersion()){
			  Logger::getLogger()->warning("Client has sent us the wrong version number (%d) than the connection is (%d)", recvframe->getVersion(), version);
			}

			rtn = true;
		} else {
			Logger::getLogger()->debug("Incorrect header");
			// protocol error
			Frame *failframe = createFrame();
			failframe->createFailFrame(fec_ProtocolError, "Protocol Error");	// TODO - should be a const or enum, protocol error
			sendFrame(failframe);
			close();
			rtn = false;
		}
	} else {
		Logger::getLogger()->debug("Did not read header");
		if (len > 0) {
			Frame *failframe = createFrame();
			failframe->createFailFrame(fec_ProtocolError, "Protocol Error");	// TODO - should be a const or enum, protocol error
			sendFrame(failframe);
		} else {
			Logger::getLogger()->info("Client disconnected");
		}
		close();
		rtn = false;
	}
	delete[] headerbuff;
	return rtn;
}
