/*  Admin Tcp Connection object, supports ipv4 and ipv6
 *
 *  Copyright (C) 2008 Aaron Mavrinac and the Thousand Parsec Project
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

#include "admintcpconn.h"

AdminTcpConnection::AdminTcpConnection() : AdminConnection(), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0), sbuff(NULL), sbuffused(0), sbuffsize(0), sendqueue(), sendandclose(false)
{

}


AdminTcpConnection::AdminTcpConnection(int fd) : AdminConnection(fd), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0), sbuff(NULL), sbuffused(0), sbuffsize(0), sendqueue(), sendandclose(false)
{
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

AdminTcpConnection::~AdminTcpConnection()
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


void AdminTcpConnection::close()
{
  if(sendqueue.empty()){
    Logger::getLogger()->debug("Closing connection");
    ::close(sockfd);
    status = 0;
  }else{
    sendandclose = true;
  }
}

void AdminTcpConnection::sendFrame(Frame * frame)
{
  if(version != frame->getVersion()){
    Logger::getLogger()->warning("Version mis-match, packet %d, connection %d", frame->getVersion(), version);

  }
  if(frame->getType() < ftad_LogMessage){  // TODO - perhaps not the best way to check
    Logger::getLogger()->error("Tried to send a non-admin frame on an admin connection, not sending frame");
  }else{
    if(status != 0 && !sendandclose){
      sendqueue.push(frame);

      processWrite();
    }
  }
}

void AdminTcpConnection::processWrite(){
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

void AdminTcpConnection::verCheck(){
  status = 2;
}

bool AdminTcpConnection::readFrame(Frame * recvframe)
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
    if (signeddatalen >= 0 && signeddatalen < 1048576) {
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
      if (type < ftad_LogMessage || type >= ftad_Max) {  // TODO - perhaps not the best way to check
        Logger::getLogger()->warning("Client has sent wrong frame type (%d)", type);
        Frame *failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_ProtocolError, "Protocol Error, non-admin frame type or frame type not known");
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

void AdminTcpConnection::sendDataAndClose(const char* data, uint32_t size){
  sendData(data, size);
  close();
}

void AdminTcpConnection::sendData(const char* data, uint32_t size){
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

int32_t AdminTcpConnection::underlyingRead(char* buff, uint32_t size){
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

int32_t AdminTcpConnection::underlyingWrite(const char* buff, uint32_t size){
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
