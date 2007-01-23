/*  Player Tcp Connection object, supports ipv4 and ipv6
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef VERSION
#define VERSION "0.0.0"
#endif


#include "logging.h"
#include "net.h"
#include "frame.h"
#include "game.h"
#include "player.h"

#include "playertcpconn.h"

PlayerTcpConnection::PlayerTcpConnection() : PlayerConnection(), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0)
{

}


PlayerTcpConnection::PlayerTcpConnection(int fd) : PlayerConnection(fd), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0)
{

}

PlayerTcpConnection::~PlayerTcpConnection()
{
	if (status != 0) {
		close();
	}
        if(rheaderbuff != NULL)
          delete[] rheaderbuff;
        if(rdatabuff != NULL)
          delete[] rdatabuff;
}


void PlayerTcpConnection::close()
{
	Logger::getLogger()->debug("Closing connection");
	if (player != NULL) {
		player->setConnection(NULL);
		player = NULL;
	}
	::close(sockfd);
	status = 0;
}

void PlayerTcpConnection::sendFrame(Frame * frame)
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



void PlayerTcpConnection::verCheck(){

  bool rtn = true;
  Frame *recvframe = new Frame(fv0_3);
  uint32_t hlen = recvframe->getHeaderLength();
  
  if(rheaderbuff == NULL){
    rbuffused = 0;
    rheaderbuff = new char[hlen];
  }
  
  if(rdatabuff == NULL && rbuffused < 4){
    int32_t len = recv(sockfd, rheaderbuff+rbuffused, 4, 0);
    if(len == 0){
      Logger::getLogger()->info("Client disconnected");
      close();
      rtn = false;
    }else if(len > 0){
      rbuffused += len;
      if(rbuffused < 4){
        Logger::getLogger()->debug("ver check header not the length needed, delaying read");
        rtn = false;
      }
    }else{
      if(errno != EAGAIN && errno != EWOULDBLOCK){
        Logger::getLogger()->warning("Socket error");
        close();
      }
      rtn = false;
    }
  }
  if(rtn && rdatabuff == NULL && rbuffused >= 4){
    if(rheaderbuff[0] == 'T' && rheaderbuff[1] == 'P'){
      //assume we have TP procotol
      if(rheaderbuff[2] == '0'){
        if(rheaderbuff[3] <= '1'){
          Logger::getLogger()->warning("Client did not show correct version of protocol (version 1)");
          send(sockfd, "You are not running the right version of TP, please upgrade\n", 60, 0);
          close();
          rtn = false;
        }else if(rheaderbuff[3] > '3'){
          //might be future version of protocol, just disconnect now
          Frame *f = new Frame(fv0_3);
          f->setSequence(0);
          f->createFailFrame(fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");
          sendFrame(f);
  
          //stay connected just in case they try again with a lower version
          // have to empty the receive queue though.
          char* buff = new char[1024];
          uint32_t len = recv(sockfd, buff, 1024, 0);
          Logger::getLogger()->debug("Read an extra %d bytes from the socket, into buffer of 1024", len);
          delete[] buff;
          rtn = false;
        }else{
          version = (FrameVersion)atoi(rheaderbuff+2);
        }
      }else{
        //might be future version of protocol, just disconnect now
        Frame *f = new Frame(fv0_3);
        f->setSequence(0);
        f->createFailFrame(fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");
        sendFrame(f);

        //stay connected just in case they try again with a lower version
        // have to empty the receive queue though.
        char* buff = new char[1024];
        uint32_t len = recv(sockfd, buff, 1024, 0);
        Logger::getLogger()->debug("Read an extra %d bytes from the socket, into buffer of 1024", len);
        delete[] buff;
        rtn = false;
      }
      
      if(rtn){
        Logger::getLogger()->info("Client has version %d of protocol", version);
        if(version != recvframe->getVersion()){
          delete recvframe;
          recvframe = new Frame(version);
        }
        if(readFrame(recvframe)){
          if(recvframe->getType() == ft02_Connect){
            char* clientsoft = recvframe->unpackString();
            Logger::getLogger()->info("Client on connection %d is [%s]", sockfd, clientsoft);
            delete[] clientsoft;
            
            status = 2;
            
            Frame *okframe = createFrame(recvframe);
            okframe->setType(ft02_OK);
            
            okframe->packString("Protocol check ok, continue! Welcome to tpserver-cpp " VERSION);
            sendFrame(okframe);
          }else if(recvframe->getVersion() >= 3 && recvframe->getType() == ft03_Features_Get){
            Logger::getLogger()->debug("Get Features request");
            Frame* features = createFrame(recvframe);

            Network::getNetwork()->createFeaturesFrame(features);

            sendFrame(features);
          }else{
            Logger::getLogger()->warning("First frame wasn't Connect or GetFeatures, was %d", recvframe->getType());
            Frame* fe = createFrame(recvframe);
            fe->createFailFrame(fec_ProtocolError, "First frame wasn't Connect (or GetFeatures in tp03), please try again");
            sendFrame(fe);
          }
        }else{
          rtn = false;
        }
      }
    }else{
      int32_t lastchance = verCheckLastChance();
      if(lastchance == 1){
        // last chance passed, try checking for frames again
        delete[] rheaderbuff;
        rheaderbuff = NULL;
      }else if(lastchance == -2){
        //waiting for more data
        rtn = false;
      }else{
        Logger::getLogger()->warning("Client did not talk any variant of TPprotocol");
        if(lastchance != 0){
          // send "I don't understand" message
          send(sockfd, "You are not running the correct protocol\n", 41, 0);
          close();
        }
        rtn = false;
      }
    }
  }else{
    rtn = false;
  }
}

int32_t PlayerTcpConnection::verCheckLastChance(){
  return -1;
}

bool PlayerTcpConnection::readFrame(Frame * recvframe)
{
  bool rtn = true;
  
  uint32_t hlen = recvframe->getHeaderLength();
  
  if(rheaderbuff == NULL){
    rbuffused = 0;
    rheaderbuff = new char[hlen];
  }
  
  if(rdatabuff == NULL && rbuffused != hlen){
    int32_t len = recv(sockfd, rheaderbuff+rbuffused, hlen - rbuffused, 0);
    if(len == 0){
      Logger::getLogger()->info("Client disconnected");
      close();
      rtn = false;
    }else if(len > 0){
      rbuffused += len;
      if(rbuffused != hlen){
        Logger::getLogger()->debug("Read header not the length needed, delaying read");
        rtn = false;
      }
    }else{
      if(errno != EAGAIN && errno != EWOULDBLOCK){
        Logger::getLogger()->warning("Socket error");
        close();
      }
      rtn = false;
    }
  }
  
  uint32_t datalen;
  
  if(rtn && ((rdatabuff == NULL && rbuffused == hlen) || rdatabuff != NULL)){
    int32_t signeddatalen;
    if ( (signeddatalen = recvframe->setHeader(rheaderbuff)) != -1 ) {
      datalen = signeddatalen;
      //sanity check
      if(version != recvframe->getVersion()){
        Logger::getLogger()->warning("Client has sent us the wrong version number (%d) than the connection is (%d)", recvframe->getVersion(), version);
      }
    }else{
      Logger::getLogger()->debug("Incorrect header");
      // protocol error
      Frame *failframe = createFrame();
      failframe->createFailFrame(fec_ProtocolError, "Protocol Error");
      sendFrame(failframe);
      close();
      rtn = false;
    }
  }
  
  if(rtn && datalen != 0){
  
    if(rdatabuff == NULL && rbuffused == hlen){
      rbuffused = 0;
      rdatabuff = new char[datalen];
    }
    
    if(rbuffused != datalen){
      int32_t len = recv(sockfd, rdatabuff+rbuffused, datalen - rbuffused, 0);
      if(len == 0){
        Logger::getLogger()->info("Client disconnected");
        close();
        rtn = false;
      }else if(len > 0){
        rbuffused += len;
        if(rbuffused != datalen){
          Logger::getLogger()->debug("Read data not the length needed, delaying read");
          rtn = false;
        }
      }else{
        if(errno != EAGAIN && errno != EWOULDBLOCK){
          Logger::getLogger()->warning("Socket error");
          close();
        }
        rtn = false;
      }
    }
    
    if(rtn && rbuffused == datalen){
      recvframe->setData(rdatabuff, datalen);
      delete[] rheaderbuff;
      delete[] rdatabuff;
      rheaderbuff = NULL;
      rdatabuff = NULL;
    }
  }else if(rtn){
    delete[] rheaderbuff;
    rheaderbuff = NULL;
  }
  return rtn;
}
