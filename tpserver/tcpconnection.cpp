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
    sendandclose( false ),
    version(fv0_3),
    paddingfilter( false )
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
  clearQueue();
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

void TcpConnection::process(){
  DEBUG("TcpConnection : About to Process");
  switch (status) {
    case PRECONNECTED:
      //check if user is really a TP protocol verNN client
      DEBUG("TcpConnection : Stage 1 - pre-connect");
      processVersionCheck();
      break;
    case CONNECTED:
      //authorise the user
      DEBUG("TcpConnection : Stage 2 - connected");
      processLogin();
      break;
    case READY:
      //process as normal
      DEBUG("TcpConnection : Stage 3 - logged in");
      processNormalFrame();
      break;
    case DISCONNECTED:
      //do nothing
      WARNING("TcpConnection : Tried to process connections that is closed or invalid");
      break;
  }
  DEBUG("TcpConnection : Finished Processing");
}

void TcpConnection::processWrite() {
  while (!sendqueue.empty()) {
    if (send_buffer.empty()) {
      send_buffer.assign( sendqueue.front()->getPacket(), sendqueue.front()->getLength() );
      send_buffer_pos = 0;
    }
    if (!send_buffer.empty()) {
      try {
        int len = underlyingWrite(send_buffer.c_str()+send_buffer_pos, send_buffer.length() - send_buffer_pos);
        if ( len <= 0 ) break;
        send_buffer_pos += len;
        if (send_buffer_pos == send_buffer.length()) {
          send_buffer.clear();
          delete sendqueue.front();
          sendqueue.pop();
        }
      } catch ( SystemException& e ) {
         ERROR("TcpConnection : Socket error writing : %s", e.what());
         clearQueue();
         close();
         return;
      }
    } else {
      ERROR("TcpConnection : Could not get packet from frame to send");
      delete sendqueue.front();
      sendqueue.pop();
      break;
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

void TcpConnection::sendString(const std::string& str){
  clearQueue();
  send_buffer = str;
  send_buffer_pos = 0;

  sendFrame(new Frame(fv0_3));
}


int32_t TcpConnection::verCheckPreChecks(){
  return 1;
}

int32_t TcpConnection::verCheckLastChance(){
  return -1;
}

bool TcpConnection::readFrame(Frame * recvframe)
{
  if (sendandclose) return false;


  uint32_t hlen = recvframe->getHeaderLength();

  if (rheaderbuff == NULL) {
    rbuffused = 0;
    rheaderbuff = new char[hlen];
  }

  if (rdatabuff == NULL && rbuffused != hlen) {
    try {
      int32_t len = underlyingRead(rheaderbuff+rbuffused, hlen - rbuffused);
      if (len == 0) {
        INFO("TcpConnection : Client disconnected");
        close();
        return false;
      } else if (len > 0) {
        rbuffused += len;
        if (rbuffused != hlen) {
          DEBUG("TcpConnection : Read header not the length needed, delaying read");
          return false;
        }
      } else {
        return false;
      }
    } catch ( SystemException& e ) {
      WARNING("TcpConnection : Socket error -- %s", e.what());
      clearQueue();
      close();
      return false;
    }
  }


  uint32_t datalen;

  if  ((rdatabuff == NULL && rbuffused == hlen) || rdatabuff != NULL) {
    int32_t signeddatalen = recvframe->setHeader(rheaderbuff);
    //check that the length field is probably valid
    // length could be negative from wire or from having no synchronisation symbol
    if (signeddatalen >= 0 && signeddatalen < 1048576) {
      datalen = signeddatalen;

    } else {
      if (signeddatalen < 1048576) {
        DEBUG("TcpConnection : Incorrect header");
        // protocol error
        sendFail( NULL, fec_ProtocolError, "Protocol Error, could not decode header");
      } else {
        DEBUG("TcpConnection : Frame too large");
        sendFail(recvframe, fec_ProtocolError, "Protocol Error, frame length too large");
      }
      close();
      return false;
    }
  }

  if (datalen != 0) {

    if (rdatabuff == NULL && rbuffused == hlen) {
      rbuffused = 0;
      rdatabuff = new char[datalen];
    }

    if (rbuffused != datalen) {
      try {
        int32_t len = underlyingRead(rdatabuff+rbuffused, datalen - rbuffused);
        if (len == 0) {
          INFO("TcpConnection : Client disconnected");
          close();
          return false;
        } else if (len > 0) {
          rbuffused += len;
          if (rbuffused != datalen) {
            DEBUG("TcpConnection : Read data not the length needed, delaying read");
            return false;
          }
        }else{
          return false;
        }
      } catch( SystemException& e ) {
        WARNING("TcpConnection : Socket error -- %s", e.what());
        clearQueue();
        close();
        return false;
      }
    }

    if(rbuffused == datalen){
      recvframe->setData(rdatabuff, datalen);
      delete[] rheaderbuff;
      delete[] rdatabuff;
      rheaderbuff = NULL;
      rdatabuff = NULL;

      //sanity checks
      if(version != recvframe->getVersion()){
        WARNING("TcpConnection : Client has sent us the wrong version number (%d) than the connection is (%d)", recvframe->getVersion(), version);
        sendFail(recvframe,fec_ProtocolError, "Protocol Error, wrong version number");
        return false;
      }
      FrameType type = recvframe->getType();
      if (type <= ft_Invalid || (version == fv0_2 && type >= ft02_Max) || (version == fv0_3 && type >= ft03_Max && type != ft04_TurnFinished) || (version == fv0_4 && type >= ft04_Max)) {
        WARNING("TcpConnection : Client has sent wrong frame type (%d)", type);
        sendFail(recvframe,fec_ProtocolError, "Protocol Error, frame type not known");
        return false;
      }
    }
  }else {
    delete[] rheaderbuff;
    rheaderbuff = NULL;
  }
  return true;
}

void TcpConnection::processVersionCheck() {

  int32_t precheck = verCheckPreChecks();
  if (precheck != 1) {
    if (precheck != -2) {
      // if not non-block wait
      if (precheck == 0) {
        INFO("TcpConnection : Client disconnected in pre-check");
      } else {
        INFO("TcpConnection : Error in pre-check, disconnecting");
      }
      close();
    }
    return;
  }

  Frame *recvframe = new Frame(fv0_3);
  uint32_t hlen = recvframe->getHeaderLength();

  if (rheaderbuff == NULL) {
    rbuffused = 0;
    rheaderbuff = new char[hlen];
  }


  if (rdatabuff == NULL && rbuffused < 4) {
    try {
      int32_t len = underlyingRead(rheaderbuff+rbuffused, 4 - rbuffused);
      if (len == 0) {
        INFO("TcpConnection : Client disconnected");
        close();
        return;
      } else if (len > 0) {
        rbuffused += len;
        if (rbuffused < 4) {
          DEBUG("TcpConnection : ver check header not the length needed, delaying read");
          return;
        }
      } else {
        return;
      }
    } catch ( SystemException& e ) {
      WARNING("TcpConnection : Socket error -- %s", e.what());
      close();
      return;
    }
  }

  if ((rdatabuff == NULL && rbuffused >= 4) || rdatabuff != NULL) {
    if (rheaderbuff[0] == 'T' && rheaderbuff[1] == 'P') {
      //assume we have TP procotol
      if (rheaderbuff[2] == '0') {
        if (rheaderbuff[3] <= '2') {
          WARNING("TcpConnection : Client did not show correct version of protocol (version 2 or less)");
          sendString("You are not running the right version of TP, please upgrade\n");
          close();
          return;
        } else if (rheaderbuff[3] > '3') {
          //might be future version of protocol, just disconnect now
          sendFail(NULL,fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");

          //stay connected just in case they try again with a lower version
          // have to empty the receive queue though.
          char* buff = new char[1024];
          try {
            int32_t len = underlyingRead(buff, 1024);
            DEBUG("TcpConnection : Read an extra %d bytes from the socket, into buffer of 1024", len);
          } catch( SystemException& ) {
            // now what?
          }
          delete[] buff;
          return;
        } else {
          char ver[] = {'\0','\0','\0'};
          memcpy(ver, rheaderbuff+2 , 2);
          int nversion = atoi(ver);
          version = (ProtocolVersion)nversion;
        }
      } else if (rheaderbuff[2] >= 4 && rheaderbuff[2] < '0') {
        //tp04 and later
        version = (ProtocolVersion)rheaderbuff[2];
        if (version > fv0_4) {
          Frame *f = new Frame(fv0_4);
          f->setSequence(0);
          f->createFailFrame(fec_ProtocolError, "TP Protocol, but I only support versions 4, sorry.");
          sendFrame(f);

          //stay connected just in case they try again with a lower version
          // have to empty the receive queue though.
          char* buff = new char[1024];
          try {
            int32_t len = underlyingRead(buff, 1024);
            DEBUG("TcpConnection : Read an extra %d bytes from the socket, into buffer of 1024", len);
          } catch( SystemException& ) {
            // now what?
          }
          delete[] buff;
          return;
        }
      }else{
        //might be future version of protocol, just disconnect now
        WARNING("TcpConnection : Unknown protocol version");

        //stay connected just in case they try again with a lower version
        // have to empty the receive queue though.
        char* buff = new char[1024];
        try {
          int32_t len = underlyingRead(buff, 1024);
          DEBUG("TcpConnection : Read an extra %d bytes from the socket, into buffer of 1024", len);
        } catch( SystemException& ) {
          // now what?
        }
        delete[] buff;

        sendFail(NULL,fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");

        delete[] rheaderbuff;
        rheaderbuff = NULL;
        rbuffused = 0;
        return;
      }

      INFO("TcpConnection : Client has version %d of protocol", version);
      if(version != recvframe->getVersion()){
        delete recvframe;
        recvframe = new Frame(version);
      }
      if (readFrame(recvframe)) {
        if (recvframe->getType() == ft02_Connect) {
          std::string clientsoft = recvframe->unpackStdString();
          INFO("TcpConnection : Client on connection %d is [%s]", sockfd, clientsoft.c_str());

          status = CONNECTED;

          Frame *okframe = createFrame(recvframe);
          okframe->setType(ft02_OK);
          okframe->packString("Protocol check ok, continue! Welcome to tpserver-cpp " VERSION);
          sendFrame(okframe);
// TODO: features before connect unsupported!
//          } else if (recvframe->getVersion() >= 3 && recvframe->getType() == ft03_Features_Get) {
//            WARNING("TcpConnection : Get Features request before Connect frame, continuing anyway");
//            processGetFeaturesFrame(recvframe);
        } else {
          WARNING("TcpConnection : First frame wasn't Connect or GetFeatures, was %d", recvframe->getType());
          sendFail(recvframe,fec_ProtocolError, "First frame wasn't Connect (or GetFeatures in tp03), please try again");
        }
      } else {
        DEBUG("TcpConnection : verCheck, did not get whole frame");
      }
    } else {
      int32_t lastchance = verCheckLastChance();
      if (lastchance == 1) {
        // last chance passed, try checking for frames again
        delete[] rheaderbuff;
        rheaderbuff = NULL;
      } else if (lastchance == -2) {
        //waiting for more data
      } else {
        WARNING("TcpConnection : Client did not talk any variant of TPprotocol");
        if (lastchance != 0) {
          // send "I don't understand" message
          sendString("You are not running the correct protocol\n");
          close();
        }
      }
    }
  }
}


void TcpConnection::sendFail(Frame* oldframe, FrameErrorCode code, const std::string& error ) {
  Frame* frame = createFrame(oldframe);
  frame->createFailFrame(code, error);
  sendFrame(frame);
}


void TcpConnection::sendSequence(Frame* oldframe, size_t sequence_size )
{
  Frame* frame = createFrame(oldframe);
  frame->setType( ft02_Sequence );
  frame->packInt( sequence_size );
  sendFrame(frame);
}

Frame* TcpConnection::createFrame(Frame* oldframe)
{
  Frame* newframe;
  if(oldframe != NULL) {
    newframe = new Frame(oldframe->getVersion());
    newframe->setSequence(oldframe->getSequence());
  } else {
    newframe = new Frame(version);
    newframe->setSequence(0);
  }
  newframe->enablePaddingStrings(paddingfilter);
  return newframe;
}

bool TcpConnection::queueEmpty() const {
  return sendqueue.empty();
}

void TcpConnection::clearQueue() {
  while (!sendqueue.empty()) {
    delete sendqueue.front();
    sendqueue.pop();
  }
}

bool TcpConnection::getAuth( Frame* frame, std::string& username, std::string& password ) {
  try{
    if(!frame->isEnoughRemaining(10))
      throw std::exception();
    username = frame->unpackStdString();
    if(!frame->isEnoughRemaining(5))
      throw std::exception();
    password = frame->unpackStdString();
  }catch(std::exception e){
    DEBUG("TcpConnection : Login - not enough data");
    sendFail(frame, fec_FrameError, "Login Error - missing username or password");
    return false;
  }
  username = username.substr(0, username.find('@'));
  if (username.length() == 0 || password.length() == 0) {
    DEBUG("TcpConnection : username or password == NULL");
    sendFail(frame,fec_FrameError, "Login Error - no username or password");	// TODO - should be a const or enum, Login error
    return false;
  }
  return true;
}