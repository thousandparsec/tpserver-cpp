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
#include "frame.h"

#include "adminconnection.h"

AdminConnection::AdminConnection(int fd) : TcpConnection(fd){
}

AdminConnection::~AdminConnection(){
  Logger::getLogger()->removeLog(logextid);
  Logger::getLogger()->info("Admin client disconnected");
}


void AdminConnection::processLogin(){
  Frame *recvframe = createFrame();
  if (readFrame(recvframe)) {
    if(recvframe->getType() == ft02_Login){
      std::string username, password;
      try{
        if(!recvframe->isEnoughRemaining(10))
          throw std::exception();
        username = recvframe->unpackStdString();
        if(!recvframe->isEnoughRemaining(5))
          throw std::exception();
        password = recvframe->unpackStdString();
      }catch(std::exception e){
        Logger::getLogger()->debug("Admin Login: not enough data");
        Frame *failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_FrameError, "Admin Login Error - missing username or password");
        sendFrame(failframe);
        delete recvframe;
        return;
      }
      username = username.substr(0, username.find('@'));
      if (username.length() > 0 && password.length() > 0) {
      bool authenticated = false;
        try{
	  if(username == Settings::getSettings()->get("admin_user") && password == Settings::getSettings()->get("admin_pass"))
            authenticated = true;
        }catch(std::exception e){
          Logger::getLogger()->debug("Admin Login: bad username or password");
          Frame *failframe = createFrame(recvframe);
          failframe->createFailFrame(fec_FrameError, "Admin Login Error - bad username or password"); // TODO - should be a const or enum, Login error
          sendFrame(failframe);
          delete recvframe;
          return;
        }
        if(authenticated){
          Frame *okframe = createFrame(recvframe);
          okframe->setType(ft02_OK);
          okframe->packString("Welcome");
          sendFrame(okframe);
          Logger::getLogger()->info("Admin login ok by %s", username.c_str());
          logsink = new AdminLogger();
	  logsink->setConnection(this);
          logextid = Logger::getLogger()->addLog(logsink);
          status = READY;
        } else {
          Logger::getLogger()->info("Bad username or password");
          Frame *failframe = createFrame(recvframe);
          failframe->createFailFrame(fec_FrameError, "Admin Login Error - bad username or password"); // TODO - should be a const or enum, Login error
          sendFrame(failframe);
        }
      } else {
              Logger::getLogger()->debug("username or password == NULL");
              Frame *failframe = createFrame(recvframe);
              failframe->createFailFrame(fec_FrameError, "Admin Login Error - no username or password");      // TODO - should be a const or enum, Login error
              sendFrame(failframe);
              //close();
      }


    }else{
      Logger::getLogger()->warning("In connected state but did not receive login");
      Frame *failframe = createFrame(recvframe);
      failframe->createFailFrame(fec_FrameError, "Wrong type of frame in this state, wanted login");
      sendFrame(failframe);
    }
  }
  delete recvframe;

}

void AdminConnection::processNormalFrame()
{
  Frame *frame = createFrame();
  if (readFrame(frame)) {
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
      Logger::getLogger()->warning("AdminConnection: Discarded frame, not processed, was type %d", frame->getType());
      Frame *of = createFrame(frame);
      of->createFailFrame(fec_ProtocolError, "Did not understand that frame type.");
      sendFrame(of);
      break;
    }
  } else {
    Logger::getLogger()->debug("noFrame :(");
    // client closed
  }
  delete frame;
}

void AdminConnection::processDescribeCommand(Frame * frame)
{
  Logger::getLogger()->debug("doing describe command frame");

  if(frame->getDataLength() < 4){
    Frame *of = createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    sendFrame(of);
    return;
  }

  int numdesc = frame->unpackInt();

  if(frame->getDataLength() < 4 + 4 * numdesc){
    Frame *of = createFrame(frame);
    of->createFailFrame(fec_FrameError, "Invalid frame");
    sendFrame(of);
  }

  if(numdesc > 1){
    Frame *seq = createFrame(frame);
    seq->setType(ft02_Sequence);
    seq->packInt(numdesc);
    sendFrame(seq);
  }

  if(numdesc == 0){
    Frame *of = createFrame(frame);
    Logger::getLogger()->debug("asked for no commands to describe");
    of->createFailFrame(fec_NonExistant, "You didn't ask for any command descriptions, try again");
    sendFrame(of);
  }

  for(int i = 0; i < numdesc; i++){
    Frame *of = createFrame(frame);
    int cmdtype = frame->unpackInt();
    CommandManager::getCommandManager()->describeCommand(cmdtype, of);
    sendFrame(of);
  }
}

void AdminConnection::processGetCommandTypes(Frame * frame){
  Logger::getLogger()->debug("doing get command types frame");

  Frame *of = createFrame(frame);
  CommandManager::getCommandManager()->doGetCommandTypes(frame, of);
  sendFrame(of);
}

void AdminConnection::processCommand(Frame * frame){
  Logger::getLogger()->debug("doing command frame");

  Frame *of = createFrame(frame);
  CommandManager::getCommandManager()->executeCommand(frame, of);
  sendFrame(of);
}
