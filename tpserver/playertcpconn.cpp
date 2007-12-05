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
#include <fcntl.h>

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

PlayerTcpConnection::PlayerTcpConnection() : PlayerConnection(), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0), sbuff(NULL), sbuffused(0), sbuffsize(0), sendqueue(), sendandclose(false)
{

}


PlayerTcpConnection::PlayerTcpConnection(int fd) : PlayerConnection(fd), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0), sbuff(NULL), sbuffused(0), sbuffsize(0), sendqueue(), sendandclose(false)
{
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
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
        if(sbuff != NULL)
          delete[] sbuff;
        while(!sendqueue.empty()){
          delete sendqueue.front();
          sendqueue.pop();
        }
}


void PlayerTcpConnection::close()
{
  if(sendqueue.empty()){
    Logger::getLogger()->debug("Closing connection");
    ::close(sockfd);
    status = 0;
  }else{
    sendandclose = true;
  }
}

void PlayerTcpConnection::sendFrame(Frame * frame)
{
  if(version != frame->getVersion()){
    Logger::getLogger()->warning("Version mis-match, packet %d, connection %d", frame->getVersion(), version);
    
  }
  if(version == fv0_2 && frame->getType() >= ft02_Max){
    Logger::getLogger()->error("Tryed to send a higher than version 2 frame on a version 2 connection, not sending frame");
  }else{
    if(status != 0 && !sendandclose){
      sendqueue.push(frame);
      
      processWrite();
    }
  }
}

void PlayerTcpConnection::processWrite(){
  bool ok = !sendqueue.empty();
  while(ok){
    if(sbuff == NULL){
      sbuff = sendqueue.front()->getPacket();
      sbuffused = 0;
      sbuffsize = sendqueue.front()->getLength();
    }
    if(sbuff != NULL){
      int len = underlyingWrite(sbuff+sbuffused, sbuffsize - sbuffused);
      if(len > 0){
        sbuffused += len;
        if(sbuffused == sbuffsize){
          delete[] sbuff;
          sbuff = NULL;
          delete sendqueue.front();
          sendqueue.pop();
          ok = !sendqueue.empty();
        }
      }else{
        ok = false;
        if(len != -2){
          Logger::getLogger()->error("Socket error writing");
          while(!sendqueue.empty()){
            delete sendqueue.front();
            sendqueue.pop();
          }
          close();
        }
      }
    }else{
      Logger::getLogger()->error("Could not get packet from frame to send");
      delete sendqueue.front();
      sendqueue.pop();
      ok = false;
    }
  }
  if(!sendqueue.empty()){
    Network::getNetwork()->addToWriteQueue(this);
  }else if(sendandclose){
    close();
  }
}


void PlayerTcpConnection::verCheck(){

  bool rtn = true;
  
  int32_t precheck = verCheckPreChecks();
  if(precheck != 1){
    rtn = false;
    if(precheck != -2){
      // if not non-block wait
      if(precheck == 0){
        Logger::getLogger()->info("Client disconnected in pre-check");
        close();
      }else{
        Logger::getLogger()->info("Error in pre-check, disconnecting");
        close();
      }
    }
  }
  if(!rtn)
    return;
  
  Frame *recvframe = new Frame(fv0_3);
  uint32_t hlen = recvframe->getHeaderLength();
  
  if(rheaderbuff == NULL){
    rbuffused = 0;
    rheaderbuff = new char[hlen];
  }
  
  if(rdatabuff == NULL && rbuffused < 4){
    int32_t len = underlyingRead(rheaderbuff+rbuffused, 4 - rbuffused);
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
      if(len != -2){
        Logger::getLogger()->warning("Socket error");
        close();
      }
      rtn = false;
    }
  }
  if(rtn && ((rdatabuff == NULL && rbuffused >= 4) || rdatabuff != NULL)){
    if(rheaderbuff[0] == 'T' && rheaderbuff[1] == 'P'){
      //assume we have TP procotol
      if(rheaderbuff[2] == '0'){
        if(rheaderbuff[3] <= '1'){
          Logger::getLogger()->warning("Client did not show correct version of protocol (version 1)");
          sendDataAndClose("You are not running the right version of TP, please upgrade\n", 60);
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
          int32_t len = underlyingRead(buff, 1024);
          Logger::getLogger()->debug("Read an extra %d bytes from the socket, into buffer of 1024", len);
          delete[] buff;
          rtn = false;
        }else{
          char ver[] = {'\0','\0','\0'};
          memcpy(ver, rheaderbuff+2 , 2);
          int nversion = atoi(ver);
          version = (FrameVersion)nversion;
        }
      }else if(rheaderbuff[2] >= 4 && rheaderbuff[2] < '0'){
        //tp04 and later
        version = (FrameVersion)rheaderbuff[2];
        if(version > fv0_4){
          Frame *f = new Frame(fv0_4);
          f->setSequence(0);
          f->createFailFrame(fec_ProtocolError, "TP Protocol, but I only support versions 4, sorry.");
          sendFrame(f);
          
          //stay connected just in case they try again with a lower version
          // have to empty the receive queue though.
          char* buff = new char[1024];
          int32_t len = underlyingRead(buff, 1024);
          Logger::getLogger()->debug("Read an extra %d bytes from the socket, into buffer of 1024", len);
          delete[] buff;
          rtn = false;
        }
      }else{
        //might be future version of protocol, just disconnect now
        Logger::getLogger()->warning("Unknown protocol version");
        
        //stay connected just in case they try again with a lower version
        // have to empty the receive queue though.
        char* buff = new char[1024];
        int32_t len = underlyingRead(buff, 1024);
        Logger::getLogger()->debug("Read an extra %d bytes from the socket, into buffer of 1024", len);
        delete[] buff;
        
        Frame *f = new Frame(fv0_3);
        f->setSequence(0);
        f->createFailFrame(fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");
        sendFrame(f);

        delete[] rheaderbuff;
        rheaderbuff = NULL;
        rbuffused = 0;
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
            std::string clientsoft = recvframe->unpackStdString();
            Logger::getLogger()->info("Client on connection %d is [%s]", sockfd, clientsoft.c_str());
            
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
          Logger::getLogger()->debug("verCheck, did not get whole frame");
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
          sendDataAndClose("You are not running the correct protocol\n", 41);
        }
        rtn = false;
      }
    }
  }else{
    rtn = false;
  }
}

int32_t PlayerTcpConnection::verCheckPreChecks(){
  return 1;
}

int32_t PlayerTcpConnection::verCheckLastChance(){
  return -1;
}

bool PlayerTcpConnection::readFrame(Frame * recvframe)
{
  bool rtn = !sendandclose;
  
  if (!rtn)
    return rtn;
  
  uint32_t hlen = recvframe->getHeaderLength();
  
  if(rheaderbuff == NULL){
    rbuffused = 0;
    rheaderbuff = new char[hlen];
  }
  
  if(rdatabuff == NULL && rbuffused != hlen){
    int32_t len = underlyingRead(rheaderbuff+rbuffused, hlen - rbuffused);
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
      if(len != -2){
        Logger::getLogger()->warning("Socket error");
        while(!sendqueue.empty()){
          delete sendqueue.front();
          sendqueue.pop();
        }
        close();
      }
      rtn = false;
    }
  }
  
  uint32_t datalen;
  
  if(rtn && ((rdatabuff == NULL && rbuffused == hlen) || rdatabuff != NULL)){
    int32_t signeddatalen = recvframe->setHeader(rheaderbuff);
    //check that the length field is probably valid
    // length could be negative from wire or from having no synchronisation symbol
    if (signeddatalen > 0 && signeddatalen < 1048576) {
      datalen = signeddatalen;
      
    }else{
      Frame *failframe;
      if(signeddatalen < 1048576) {
        Logger::getLogger()->debug("Incorrect header");
        // protocol error
        failframe = createFrame();
        failframe->createFailFrame(fec_ProtocolError, "Protocol Error, could not decode header");
      }else{
        Logger::getLogger()->debug("Frame too large");
        failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_ProtocolError, "Protocol Error, frame length too large");
      }
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
      int32_t len = underlyingRead(rdatabuff+rbuffused, datalen - rbuffused);
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
        if(len != -2){
          Logger::getLogger()->warning("Socket error");
          while(!sendqueue.empty()){
            delete sendqueue.front();
            sendqueue.pop();
          }
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
      
      //sanity checks
      if(version != recvframe->getVersion()){
        Logger::getLogger()->warning("Client has sent us the wrong version number (%d) than the connection is (%d)", recvframe->getVersion(), version);
        Frame *failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_ProtocolError, "Protocol Error, wrong version number");
        sendFrame(failframe);
        rtn = false;
      }
      FrameType type = recvframe->getType();
      if (type <= ft_Invalid || (version == fv0_2 && type >= ft02_Max) || (version == fv0_3 && type >= ft03_Max && type != ft04_TurnFinished) || (version == fv0_4 && type >= ft04_Max)) {
        Logger::getLogger()->warning("Client has sent wrong frame type (%d)", type);
        Frame *failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_ProtocolError, "Protocol Error, frame type not known");
        sendFrame(failframe);
        rtn = false;
      }
    }
  }else if(rtn){
    delete[] rheaderbuff;
    rheaderbuff = NULL;
  }
  return rtn;
}

void PlayerTcpConnection::sendDataAndClose(const char* data, uint32_t size){
  sendData(data, size);
  close();
}

void PlayerTcpConnection::sendData(const char* data, uint32_t size){
  while(!sendqueue.empty()){
    delete sendqueue.front();
    sendqueue.pop();
  }
  sbuff = new char[size];
  memcpy(sbuff, data, size);
  sbuffsize = size;
  sbuffused = 0;
  sendFrame(new Frame(fv0_3));
}

int32_t PlayerTcpConnection::underlyingRead(char* buff, uint32_t size){
  int32_t len = recv(sockfd, buff, size, 0);
  if(len < 0){
    if(errno != EAGAIN && errno != EWOULDBLOCK){
      Logger::getLogger()->error("underlying read, tcp, error is: %s", strerror(errno));
      len = -1;
    }else{
      len = -2;
    }
  }
  return len;
}

int32_t PlayerTcpConnection::underlyingWrite(const char* buff, uint32_t size){
  int len = send(sockfd, buff, size, 0);
  if(len < 0){
    if(errno != EAGAIN && errno != EWOULDBLOCK){
      Logger::getLogger()->error("underlying write, tcp, error is: %s", strerror(errno));
      len = -1;
    }else{
      len = -2;
    }
  }
  return len;
}
