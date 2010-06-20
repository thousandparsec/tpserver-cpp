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

#include "systemexception.h"
#include "tcpconnection.h"

TcpConnection::TcpConnection(int fd, Type aType) 
  : Connection(fd,aType), 
    read_buffer_pos( 0 ),
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
  try {
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
  } catch ( FrameException& exception ) {
    // Control should never get here -- this is just so FrameExceptions don't blow up the server
    ERROR("TcpConnection : unhandled frame exception caught : %s", exception.what() );
  }
  DEBUG("TcpConnection : Finished Processing");
}

void TcpConnection::processWrite() {
  while (!sendqueue.empty()) {
    if (send_buffer.empty()) {
      send_buffer = sendqueue.front()->getPacket();
      send_buffer_pos = 0;
    }
    if (!send_buffer.empty()) {
      try {
        int len = underlyingWrite(send_buffer.c_str()+send_buffer_pos, send_buffer.length() - send_buffer_pos);
        if ( len <= 0 ) break;
        send_buffer_pos += len;
        if (send_buffer_pos == send_buffer.length()) {
          send_buffer.clear();
          sendqueue.pop();
        }
      } catch ( SystemException& e ) {
         clearQueue();
         close();
         ERROR("TcpConnection : Socket error writing : %s", e.what());
         return;
      }
    } else {
      ERROR("TcpConnection : Could not get packet from frame to send");
      sendqueue.pop();
      break;
    }
  }
  if (!sendqueue.empty()) {
    Network::getNetwork()->addToWriteQueue(shared_from_this());
  } else if (sendandclose) {
    close();
  }
}

void TcpConnection::sendFrame( OutputFrame::Ptr frame )
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
  int len = ::send(sockfd, buff, size, 0);
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

  sendFrame(OutputFrame::Ptr(new OutputFrame(fv0_3)));
}


int32_t TcpConnection::verCheckPreChecks(){
  return 1;
}

int32_t TcpConnection::verCheckLastChance(){
  return -1;
}

bool TcpConnection::readFrame(InputFrame::Ptr recvframe)
{
  if (sendandclose) return false;


  uint32_t hlen = recvframe->getHeaderLength();

  if (header_buffer.empty()) {
    read_buffer_pos = 0;
    header_buffer.resize( hlen );
  }

  if (data_buffer.empty() && read_buffer_pos != hlen) {
    try {
      // Yes, yes, I know, it's bad, it's non-standard conforming. But damn it works.
      int32_t len = underlyingRead(const_cast<char*>(header_buffer.data())+read_buffer_pos, hlen - read_buffer_pos);
      if (len == 0) {
        INFO("TcpConnection : Client disconnected");
        close();
        return false;
      } else if (len > 0) {
        read_buffer_pos += len;
        if (read_buffer_pos != hlen) {
          DEBUG("TcpConnection : Read header not the length needed, delaying read");
          return false;
        }
      } else {
        return false;
      }
    } catch ( SystemException& e ) {
      clearQueue();
      close();
      WARNING("TcpConnection : Socket error -- %s", e.what());
      return false;
    }
  }


  uint32_t datalen;

  if  ((data_buffer.empty() && read_buffer_pos == hlen) || !data_buffer.empty() ) {
    int32_t signeddatalen = recvframe->setHeader(header_buffer);
    //check that the length field is probably valid
    // length could be negative from wire or from having no synchronisation symbol
    if (signeddatalen >= 0 && signeddatalen < 1048576) {
      datalen = signeddatalen;

    } else {
      if (signeddatalen < 1048576) {
        DEBUG("TcpConnection : Incorrect header");
        // protocol error
        sendFail( InputFrame::Ptr(), fec_ProtocolError, "Protocol Error, could not decode header");
      } else {
        DEBUG("TcpConnection : Frame too large");
        sendFail(recvframe, fec_ProtocolError, "Protocol Error, frame length too large");
      }
      close();
      return false;
    }
  }

  if (datalen != 0) {

    if (data_buffer.empty() && read_buffer_pos == hlen) {
      read_buffer_pos = 0;
      data_buffer.resize(datalen);
    }

    if (read_buffer_pos != datalen) {
      try {
        // Yes, yes, I know, it's bad, it's non-standard conforming. But damn it works.
        int32_t len = underlyingRead(const_cast<char*>(data_buffer.data())+read_buffer_pos, datalen - read_buffer_pos);
        if (len == 0) {
          INFO("TcpConnection : Client disconnected");
          close();
          return false;
        } else if (len > 0) {
          read_buffer_pos += len;
          if (read_buffer_pos != datalen) {
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

    if(read_buffer_pos == datalen){
      recvframe->setData(data_buffer);
      header_buffer.clear();
      data_buffer.clear();

      //sanity checks
      if(version != recvframe->getVersion()){
        WARNING("TcpConnection : Client has sent us the wrong version number (%d) than the connection is (%d)", recvframe->getVersion(), version);
        sendFail(recvframe,fec_ProtocolError, "Protocol Error, wrong version number");
        return false;
      }
      
    }
  }else{
    header_buffer.clear();
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

  InputFrame::Ptr recvframe( new InputFrame() );
  uint32_t hlen = recvframe->getHeaderLength();

  if (header_buffer.empty()) {
    read_buffer_pos = 0;
    header_buffer.resize(hlen);
  }


  if (data_buffer.empty() && read_buffer_pos < 4) {
    try {
      // Yes, yes, I know, it's bad, it's non-standard conforming. But damn it works.
      int32_t len = underlyingRead(const_cast<char*>(header_buffer.data())+read_buffer_pos, 4 - read_buffer_pos);
      if (len == 0) {
        INFO("TcpConnection : Client disconnected");
        close();
        return;
      } else if (len > 0) {
        read_buffer_pos += len;
        if (read_buffer_pos < 4) {
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

  if ((data_buffer.empty() && read_buffer_pos >= 4) || !data_buffer.empty()) {
    if ( header_buffer.compare(0,2,"TP") == 0 ) {
      //assume we have TP procotol
      if ( header_buffer[2] == '0') {
        if ( header_buffer[3] <= '2') {
          WARNING("TcpConnection : Client did not show correct version of protocol (version 2 or less)");
          sendString("You are not running the right version of TP, please upgrade\n");
          close();
          return;
        } else if ( header_buffer[3] > '3') {
          //might be future version of protocol, just disconnect now
          sendFail(InputFrame::Ptr(),fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");

          //stay connected just in case they try again with a lower version
          // have to empty the receive queue though.
          char buff[1024];
          try {
            int32_t len = underlyingRead(&buff[0], 1024);
            DEBUG("TcpConnection : Read an extra %d bytes from the socket, into buffer of 1024", len);
          } catch( SystemException& ) {
            // now what?
          }
          return;
        } else {
          char ver[] = {'\0','\0','\0'};
          memcpy(ver, header_buffer.c_str()+2 , 2);
          int nversion = atoi(ver);
          version = (ProtocolVersion)nversion;
        }
      } else if (header_buffer[2] >= 4 && header_buffer[2] < '0') {
        //tp04 and later
        version = (ProtocolVersion)header_buffer[2];
        if (version > fv0_4) {
          sendFail( InputFrame::Ptr() ,fec_ProtocolError, "TP Protocol, but I only support versions 4, sorry.");

          //stay connected just in case they try again with a lower version
          // have to empty the receive queue though.
          char buff[1024];
          try {
            int32_t len = underlyingRead(&buff[0], 1024);
            DEBUG("TcpConnection : Read an extra %d bytes from the socket, into buffer of 1024", len);
          } catch( SystemException& ) {
            // now what?
          }
          return;
        }
      }else{
        //might be future version of protocol, just disconnect now
        WARNING("TcpConnection : Unknown protocol version");

        //stay connected just in case they try again with a lower version
        // have to empty the receive queue though.
        char buff[1024];
        try {
          int32_t len = underlyingRead(&buff[0], 1024);
          DEBUG("TcpConnection : Read an extra %d bytes from the socket, into buffer of 1024", len);
        } catch( SystemException& ) {
          // now what?
        }

        sendFail(InputFrame::Ptr(),fec_ProtocolError, "TP Protocol, but I only support versions 2 and 3, sorry.");

        header_buffer.clear();
        read_buffer_pos = 0;
        return;
      }

      INFO("TcpConnection : Client has version %d of protocol", version);
      if(version != recvframe->getVersion()){
        recvframe.reset( new InputFrame(version) );
      }
      if (readFrame(recvframe)) {
        try {
          if (recvframe->getType() == ft02_Connect) {
            std::string clientsoft = recvframe->unpackString();

            INFO("TcpConnection : Client on connection %d is [%s]", sockfd, clientsoft.c_str());

            status = CONNECTED;

            sendOK(recvframe,"Protocol check ok, continue! Welcome to tpserver-cpp " VERSION);
            // TODO: features before connect unsupported!
            //          } else if (recvframe->getVersion() >= 3 && recvframe->getType() == ft03_Features_Get) {
            //            WARNING("TcpConnection : Get Features request before Connect frame, continuing anyway");
            //            processGetFeaturesFrame(recvframe);
          } else {
            throw FrameException( fec_ProtocolError, "First frame wasn't Connect (or GetFeatures in tp03), please try again" );
          }
        } catch ( FrameException& exception ) {
          WARNING( "TcpConnection first FrameException : %s", exception.what() );
          sendFail( recvframe, exception.getErrorCode(), exception.getErrorMessage() );
        }
      } else {
        DEBUG("TcpConnection : verCheck, did not get whole frame");
      }
    } else {
      int32_t lastchance = verCheckLastChance();
      if (lastchance == 1) {
        // last chance passed, try checking for frames again
        header_buffer.clear();
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


void TcpConnection::sendFail(InputFrame::Ptr oldframe, FrameErrorCode code, const std::string& error, const RefList& reflist ) {
  OutputFrame::Ptr frame = createFrame( oldframe );
  frame->setType( ft02_Fail );
  frame->packInt( code );
  frame->packString( error );
  if ( frame->getVersion() >= fv0_4 ){
    frame->packInt(reflist.size());
    for (RefList::const_iterator ref = reflist.begin(); ref != reflist.end(); ++ref){
      frame->packInt(ref->first);
      frame->packInt(ref->second);
    }
  }
  sendFrame(frame);
}

void TcpConnection::sendFail(InputFrame::Ptr oldframe, const FrameException& fe){
    sendFail(oldframe, fe.getErrorCode(), fe.getErrorMessage(), fe.getRefList());
}

void TcpConnection::sendSequence(InputFrame::Ptr oldframe, size_t sequence_size )
{
  OutputFrame::Ptr frame = createFrame(oldframe);
  frame->setType( ft02_Sequence );
  frame->packInt( sequence_size );
  sendFrame(frame);
}

void TcpConnection::send(InputFrame::Ptr oldframe, const Packable* packable )
{
  OutputFrame::Ptr frame = createFrame(oldframe);
  packable->pack( frame );
  sendFrame(frame);
}

void TcpConnection::send(InputFrame::Ptr oldframe, const Packable::Ptr packable )
{
  OutputFrame::Ptr frame = createFrame(oldframe);
  packable->pack( frame );
  sendFrame(frame);
}

void TcpConnection::sendOK(InputFrame::Ptr oldframe, const std::string& message )
{
  OutputFrame::Ptr frame = createFrame(oldframe);
  frame->setType( ft02_OK );
  frame->packString( message );
  sendFrame(frame);
}

void TcpConnection::sendModList(InputFrame::Ptr oldframe, FrameType ft, uint32_t sequence, const IdModList& modlist, uint32_t count, uint32_t start, uint64_t fromtime ) 
{
  if(start > modlist.size()){
    DEBUG("Starting number too high, snum = %d, size = %d", start, modlist.size());
    sendFail(oldframe,fec_NonExistant, "Starting number too high");
    return;
  }
  if(count > modlist.size() - start){
    count = modlist.size() - start;
  }

  if(count > MAX_ID_LIST_SIZE + ((oldframe->getVersion() < fv0_4)? 1 : 0)){
    DEBUG("Number of items to get too high, numtoget = %d", count);
    sendFail(oldframe,fec_FrameError, "Too many items to get, frame too big");
    return;
  }
  OutputFrame::Ptr frame = createFrame(oldframe);
  frame->setType(ft);
  frame->packInt(sequence);
  frame->packIdModList(modlist,count,start);
  if (frame->getVersion() >= fv0_4) {
    frame->packInt64(fromtime);
  }
  sendFrame(frame);
}

OutputFrame::Ptr TcpConnection::createFrame(InputFrame::Ptr oldframe)
{
  OutputFrame::Ptr newframe;
  if(oldframe != NULL) {
    newframe.reset( new OutputFrame(oldframe->getVersion(),paddingfilter) );
    newframe->setSequence(oldframe->getSequence());
  } else {
    newframe.reset( new OutputFrame(version,paddingfilter) );
  }
  return newframe;
}

bool TcpConnection::queueEmpty() const {
  return sendqueue.empty();
}

void TcpConnection::clearQueue() {
  while ( !sendqueue.empty() ) sendqueue.pop();
}

bool TcpConnection::getAuth( InputFrame::Ptr frame, std::string& username, std::string& password ) {
  try{
    if(!frame->isEnoughRemaining(15)) throw FrameException( fec_FrameError );
    username = frame->unpackString();
    password = frame->unpackString();
  }catch( FrameException& e ){
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

std::string TcpConnection::getHeader() const {
  return header_buffer;
}
