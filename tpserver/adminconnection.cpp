/*  Admin Connection object
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "settings.h"
#include "logging.h"
#include "adminlogger.h"
#include "net.h"
#include "commandmanager.h"

#include "adminconnection.h"

AdminConnection::AdminConnection(int fd) : TcpConnection(fd,ADMIN){
}

AdminConnection::~AdminConnection(){
  Logger::getLogger()->removeLog(logextid);
  Logger::getLogger()->info("Admin client disconnected");
}


void AdminConnection::processLogin(){
  InputFrame *recvframe = new InputFrame(version, paddingfilter);
  if (readFrame(recvframe)) {
    try {
      if(recvframe->getType() == ft02_Login){
        std::string username, password;

        if ( getAuth( recvframe, username, password ) ) {

          bool authenticated = false;
          try{
            if(username == Settings::getSettings()->get("admin_user") && password == Settings::getSettings()->get("admin_pass"))
              authenticated = true;
          }catch(std::exception e){
          }
          if(authenticated){
            sendOK( recvframe, "Welcome" );
            Logger::getLogger()->info("Admin login ok by %s", username.c_str());
            logsink = new AdminLogger();
            logsink->setConnection(this);
            logextid = Logger::getLogger()->addLog(logsink);
            status = READY;
          } else {
            throw FrameException( fec_FrameError, "Admin Login Error - bad username or password"); // TODO - should be a const or enum, Login error
          }
        }

      }else{
        throw FrameException( fec_FrameError, "Wrong type of frame in this state, wanted login");
      }
    } catch ( FrameException& exception ) {
      // This might be overkill later, but now let's log it
      DEBUG( "AdminConnection caught FrameException : %s", exception.what() );
      sendFail( recvframe, exception.getErrorCode(), exception.getErrorMessage() );
    }
  }
  delete recvframe;

}

void AdminConnection::processNormalFrame()
{
  InputFrame *frame = new InputFrame(version,paddingfilter);
  if (readFrame(frame)) {
    try {
      switch (frame->getType()) {
        case ftad_CommandDesc_Get:
          processDescribeCommand(frame);
          break;
        case ftad_CommandTypes_Get:
          processGetCommandTypes(frame);
          break;
        case ftad_Command:
          processCommand(frame);
          break;
        default:
          WARNING("AdminConnection: Discarded frame, not processed, was type %d", frame->getType());
          throw FrameException( fec_ProtocolError, "Did not understand that frame type.");
          break;
      }
    } catch ( FrameException& exception ) {
      // This might be overkill later, but now let's log it
      DEBUG( "AdminConnection caught FrameException : %s", exception.what() );
      sendFail( frame, exception.getErrorCode(), exception.getErrorMessage() );
    }
  } else {
    DEBUG("noFrame :(");
    // client closed
  }
  delete frame;
}

void AdminConnection::processDescribeCommand(InputFrame * frame)
{
  Logger::getLogger()->debug("doing describe command frame");

  if(frame->getDataLength() < 4){
    throw FrameException(fec_FrameError);
  }

  int numdesc = frame->unpackInt();

  if(frame->getDataLength() < 4 + 4 * numdesc){
    throw FrameException(fec_FrameError);
  }

  if(numdesc > 1){
    sendSequence(frame,numdesc);
  }

  if(numdesc == 0){
    DEBUG("asked for no commands to describe");
    throw FrameException( fec_NonExistant, "You didn't ask for any command descriptions, try again");
  }

  for(int i = 0; i < numdesc; i++){
    OutputFrame::Ptr of = createFrame(frame);
    int cmdtype = frame->unpackInt();
    CommandManager::getCommandManager()->describeCommand(cmdtype, of);
    sendFrame(of);
  }
}

void AdminConnection::processGetCommandTypes(InputFrame * frame){
  DEBUG("doing get command types frame");

  OutputFrame::Ptr of = createFrame(frame);
  CommandManager::getCommandManager()->doGetCommandTypes(frame, of);
  sendFrame(of);
}

void AdminConnection::processCommand(InputFrame * frame){
  DEBUG("doing command frame");

  OutputFrame::Ptr of = createFrame(frame);
  CommandManager::getCommandManager()->executeCommand(frame, of);
  sendFrame(of);
}
