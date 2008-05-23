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

#include "logging.h"
#include "net.h"
#include "frame.h"

#include "adminconnection.h"

AdminConnection::AdminConnection() : Connection(), version(fv0_4){
}


AdminConnection::AdminConnection(int fd) : Connection(){
  sockfd = fd;
  status = 1;
}

AdminConnection::~AdminConnection(){
}


void AdminConnection::setFD(int fd){
  sockfd = fd;
  status = 1;
}

void AdminConnection::process(){
  Logger::getLogger()->debug("About to Process");
  switch (status) {
  case 1:
    //check if user is really an admin client
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
    adminFrame();
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

Frame* AdminConnection::createFrame(Frame* oldframe)
{
  Frame* newframe;
  if(oldframe != NULL) {
    newframe = new Frame(oldframe->getVersion());
    newframe->setSequence(oldframe->getSequence());
  } else {
    newframe = new Frame(version);
    newframe->setSequence(0);
  }
  newframe->enablePaddingStrings(false);  // TODO - what does this do?
  return newframe;
}

void AdminConnection::login(){
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
      int temp_auth = 0; // TODO - this should be a useful object
        try{
	  //TODO - real authentication
	  if(username == "admin" && password == "admin")
            temp_auth = 1;
        }catch(std::exception e){
          Logger::getLogger()->debug("Admin Login: bad username or password");
          Frame *failframe = createFrame(recvframe);
          failframe->createFailFrame(fec_FrameError, "Admin Login Error - bad username or password"); // TODO - should be a const or enum, Login error
          sendFrame(failframe);
          delete recvframe;
          return;
        }
        if(temp_auth){
          Frame *okframe = createFrame(recvframe);
          okframe->setType(ft02_OK);
          okframe->packString("Welcome");
          sendFrame(okframe);
          Logger::getLogger()->info("Admin login ok by %s", username.c_str());
          status = 3;
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

void AdminConnection::adminFrame()
{
  Frame *frame = createFrame();
  if (readFrame(frame)) {
    // TODO - do stuff with frames
  } else {
    Logger::getLogger()->debug("noFrame :(");
    // client closed
  }
  delete frame;
}
