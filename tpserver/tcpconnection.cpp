/*  Tcp Connection object, supports ipv4 and ipv6
 *
 *  Copyright (C) 2009 Kornel Kisielewicz and the Thousand Parsec Project
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
#include <stdint.h>
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

#include "systemexception.h"
#include "tcpconnection.h"

TcpConnection::TcpConnection(int fd) 
  : Connection(fd), 
    rheaderbuff( NULL ),
    rdatabuff( NULL ),
    rbuffused( 0 ),
    sbuff( NULL ),
    sbuffused( 0 ),
    sbuffsize( 0 ),
    sendandclose( false ),
    version(fv0_3)
{
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

TcpConnection::~TcpConnection() {
  if (status != DISCONNECTED) {
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

void TcpConnection::close()
{
  if(sendqueue.empty()){
    ::close(sockfd);
    status = DISCONNECTED;
  }else{
    sendandclose = true;
  }
}

void TcpConnection::processWrite() {
  bool ok = !sendqueue.empty();
  while (ok) {
    if (sbuff == NULL) {
      sbuff = sendqueue.front()->getPacket();
      sbuffused = 0;
      sbuffsize = sendqueue.front()->getLength();
    }
    if (sbuff != NULL) {
      try {
        int len = underlyingWrite(sbuff+sbuffused, sbuffsize - sbuffused);
        if (len > 0) {
          sbuffused += len;
          if (sbuffused == sbuffsize) {
            delete[] sbuff;
            sbuff = NULL;
            delete sendqueue.front();
            sendqueue.pop();
            ok = !sendqueue.empty();
          }
        } else {
          ok = false;
        }
      } catch ( SystemException& e ) {
         ERROR("TcpConnection : Socket error writing : %s", e.what());
         while (!sendqueue.empty()) {
           delete sendqueue.front();
           sendqueue.pop();
         }
         close();
         return;
      }
    } else {
      ERROR("TcpConnection : Could not get packet from frame to send");
      delete sendqueue.front();
      sendqueue.pop();
      ok = false;
    }
  }
  if (!sendqueue.empty()) {
    Network::getNetwork()->addToWriteQueue(this);
  } else if (sendandclose) {
    close();
  }
}

void TcpConnection::sendFrame( Frame* frame )
{
  if (version != frame->getVersion()) {
    WARNING("TcpConnection : Version mis-match, packet %d, connection %d", frame->getVersion(), version);
  }
  if (version == fv0_2 && frame->getType() >= ft02_Max) {
    ERROR("TcpConnection : Tried to send a higher than version 2 frame on a version 2 connection, not sending frame");
  } else {
    if (status != DISCONNECTED && !sendandclose) {
      sendqueue.push(frame);
      processWrite();
    }
  }
}

ProtocolVersion TcpConnection::getProtocolVersion()
{
  return version;
}

int32_t TcpConnection::underlyingRead(char* buff, uint32_t size) {
  int32_t len = recv(sockfd, buff, size, 0);
  if(len < 0){
    if(errno == EAGAIN || errno == EWOULDBLOCK){
      return -2;
    }else{
      throw SystemException();
    }
  }
  return len;
}
int32_t TcpConnection::underlyingWrite(const char* buff, uint32_t size) {
  int len = send(sockfd, buff, size, 0);
  if(len < 0){
    if(errno == EAGAIN || errno == EWOULDBLOCK){
      len = -2;
    }else{
      throw SystemException();
    }
  }
  return len;
}

void TcpConnection::sendDataAndClose(const char* data, uint32_t size){
  sendData(data, size);
  close();
}

void TcpConnection::sendData(const char* data, uint32_t size){
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


int32_t TcpConnection::verCheckPreChecks(){
  return 1;
}

int32_t TcpConnection::verCheckLastChance(){
  return -1;
}

