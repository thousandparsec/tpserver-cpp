/*  Player Connection object
 *
 *  Copyright (C) 2003-2005,2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "net.h"
#include "frame.h"
#include "game.h"
#include "player.h"
#include "playermanager.h"
#include "settings.h"
#include "playeragent.h"
#include "turntimer.h"

#include "playerconnection.h"

PlayerConnection::PlayerConnection() : Connection(), playeragent(NULL), version(fv0_3), paddingfilter(false){
  lastpingtime = time(NULL);
}


PlayerConnection::PlayerConnection(int fd) : Connection(), playeragent(NULL), version(fv0_3), paddingfilter(false){
  sockfd = fd;
  status = 1;
  lastpingtime = time(NULL);
}

PlayerConnection::~PlayerConnection(){
  if(playeragent != NULL){
    delete playeragent;
  }
}


void PlayerConnection::setFD(int fd){
  sockfd = fd;
  status = 1;
}

void PlayerConnection::process(){
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


Frame* PlayerConnection::createFrame(Frame* oldframe)
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


void PlayerConnection::login(){
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
        Logger::getLogger()->debug("Login: not enough data");
        Frame *failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_FrameError, "Login Error - missing username or password");
        sendFrame(failframe);
        delete recvframe;
        return;
      }
      username = username.substr(0, username.find('@'));
      if (username.length() > 0 && password.length() > 0) {
        Player* player = NULL;
        try{
          player = Game::getGame()->getPlayerManager()->findPlayer(username, password);
        }catch(std::exception e){
          if(Settings::getSettings()->get("autoadd_players") == "yes" &&
                Settings::getSettings()->get("add_players") == "yes") {
                Logger::getLogger()->info("Creating new player automatically");
              player = Game::getGame()->getPlayerManager()->createNewPlayer(username, password);
              if(player != NULL){
                  Frame* f = createFrame(recvframe);
                  f->createFailFrame(fec_PermissionDenied, "Cannot create new player");
                  sendFrame(f);
                  return;
              }
          }
        }
        if(player != NULL){
          Frame *okframe = createFrame(recvframe);
          okframe->setType(ft02_OK);
          okframe->packString("Welcome");
          sendFrame(okframe);
          Logger::getLogger()->info("Login ok by %s", username.c_str());
          playeragent = new PlayerAgent();
          playeragent->setPlayer(player);
          playeragent->setConnection(this);
          status = 3;
        } else {
          Logger::getLogger()->info("Bad username or password");
          Frame *failframe = createFrame(recvframe);
          failframe->createFailFrame(fec_FrameError, "Login Error - bad username or password");	// TODO - should be a const or enum, Login error
          sendFrame(failframe);
        }
      } else {
              Logger::getLogger()->debug("username or password == NULL");
              Frame *failframe = createFrame(recvframe);
              failframe->createFailFrame(fec_FrameError, "Login Error - no username or password");	// TODO - should be a const or enum, Login error
              sendFrame(failframe);
              //close();
      }


    }else if(version >= fv0_3 && recvframe->getType() == ft03_Features_Get){
      processGetFeaturesFrame(recvframe);
    }else if(recvframe->getType() == ft02_Time_Remaining_Get){
      processTimeRemainingFrame(recvframe);
    }else if(version >= fv0_3 && recvframe->getType() == ft03_Account){
      if(Settings::getSettings()->get("add_players") == "yes"){
        std::string username = recvframe->unpackStdString();
        std::string password = recvframe->unpackStdString();
        username = username.substr(0, username.find('@'));
        if (username.length() > 0 && password.length() > 0) {
          Logger::getLogger()->info("Creating new player");
          Player* player = Game::getGame()->getPlayerManager()->createNewPlayer(username, password);
          if(player != NULL){
            // also email address and comment strings
            player->setEmail(recvframe->unpackStdString());
            player->setComment(recvframe->unpackStdString());
            Frame *okframe = createFrame(recvframe);
            okframe->setType(ft02_OK);
            okframe->packString("Account created.");
            sendFrame(okframe);
            Logger::getLogger()->info("Account created ok for %s", username.c_str());
            playeragent = new PlayerAgent();
            playeragent->setPlayer(player);
            playeragent->setConnection(this);
          }else{
            Logger::getLogger()->info("Bad username or password in account creation");
            Frame *failframe = createFrame(recvframe);
            failframe->createFailFrame(fec_FrameError, "Account creation Error - bad username or password");	// TODO - should be a const or enum, Login error
            sendFrame(failframe);
          }
        }else{
          Logger::getLogger()->debug("username or password == NULL in account frame");
          Frame *failframe = createFrame(recvframe);
          failframe->createFailFrame(fec_FrameError, "Account Error - no username or password");	// TODO - should be a const or enum, Login error
          sendFrame(failframe);
          close();
        }
      }else{
        Logger::getLogger()->info("Account creation disabled, not creating account");
        Frame *failframe = createFrame(recvframe);
        failframe->createFailFrame(fec_PermissionDenied, "Account creation Disabled, talk to game admin");
        sendFrame(failframe);
      }
    }else if(version >= fv0_4 && recvframe->getType() == ft04_GameInfo_Get){
      processGetGameInfoFrame(recvframe);
    }else if(version >= fv0_4 && recvframe->getType() == ft04_Filters_Set){
      processSetFilters(recvframe);
    }else{
      Logger::getLogger()->warning("In connected state but did not receive login, get features or get get time remaining");
      Frame *failframe = createFrame(recvframe);
      failframe->createFailFrame(fec_FrameError, "Wrong type of frame in this state, wanted login, account or get features");
      sendFrame(failframe);
    }
  }
  delete recvframe;

}

void PlayerConnection::inGameFrame()
{
  Frame *frame = createFrame();
  if (readFrame(frame)) {
    
    if(version >= fv0_3 && frame->getType() == ft03_Features_Get){
      processGetFeaturesFrame(frame);
    }else if(version >= fv0_3 && frame->getType() == ft03_Ping){
      Logger::getLogger()->debug("Processing Ping frame");
      // check for the time of the last frame, ignore this if
      //  less than 60 seconds ago.
      if(lastpingtime < static_cast<uint64_t>(time(NULL)) - 60){
        lastpingtime = time(NULL);
        Frame *pong = createFrame(frame);
        pong->setType(ft02_OK);
        pong->packString("Keep alive ok, hope you're still there");
        sendFrame(pong);
        Logger::getLogger()->debug("Did ping for client %d", sockfd);
      }else{
        Logger::getLogger()->warning("Client %d tried to ping within 60 seconds of the last ping", sockfd);
      }
    }else if(frame->getType() == ft02_Time_Remaining_Get){
            processTimeRemainingFrame(frame);
    }else if(version >= fv0_4 && frame->getType() == ft04_GameInfo_Get){
            processGetGameInfoFrame(frame);
    }else if(version >= fv0_4 && frame->getType() == ft04_Filters_Set){
            processSetFilters(frame);
    }else{
      if(Game::getGame()->isStarted()){
        // should pass frame to player to do something with
        Logger::getLogger()->debug("inGameFrame");
        playeragent->processIGFrame(frame);
      }else{
        Frame *gns = createFrame(frame);
        gns->createFailFrame(fec_TempUnavailable, "Game has not yet started, please wait");
        sendFrame(gns);
      }
    }
  } else {
    Logger::getLogger()->debug("noFrame :(");
    // client closed
  }
  delete frame;
}

void PlayerConnection::processGetFeaturesFrame(Frame* frame){
    Logger::getLogger()->debug("Processing Get Features frame");
    Frame *features = createFrame(frame);
    features->setType(ft03_Features);
    std::set<uint32_t> fids;
    fids.insert(fid_keep_alive);
    fids.insert(fid_serverside_property);
    if(frame->getVersion() >= fv0_4){
        fids.insert(fid_filter_stringpad);
    }
    if(Settings::getSettings()->get("add_players") == "yes"){
        fids.insert(fid_account_register);
    }
    
    features->packInt(fids.size());
    for(std::set<uint32_t>::iterator itcurr = fids.begin(); itcurr != fids.end(); ++itcurr){
      features->packInt(*itcurr);
    }
    sendFrame(features);
}

void PlayerConnection::processGetGameInfoFrame(Frame* frame){
    Logger::getLogger()->debug("Processing get GameInfo frame");
    Frame *game = createFrame(frame);
    Game::getGame()->packGameInfoFrame(game);
    sendFrame(game);
}

void PlayerConnection::processSetFilters(Frame* frame){
    Logger::getLogger()->debug("Processing set filters");
    Frame* rtnframe = createFrame(frame);
    uint32_t numfilters = frame->unpackInt();
    std::set<uint32_t> filters_wanted;
    for(uint32_t i = 0; i < numfilters; i++){
        filters_wanted.insert(frame->unpackInt());
    }

    std::set<uint32_t> filters_setup;
    if(filters_wanted.count(fid_filter_stringpad) != 0){
        filters_setup.insert(fid_filter_stringpad);
    }

    if(filters_wanted.size() != filters_setup.size()){
        rtnframe->createFailFrame(fec_PermUnavailable, "Not all filters specified are available");
        sendFrame(rtnframe);
    }else{
        rtnframe->setType(ft02_OK);
        rtnframe->packString("Filters ready, setting filters now");
        sendFrame(rtnframe);
        if(filters_setup.count(fid_filter_stringpad) != 0){
            paddingfilter = true;
        }
    }
}

void PlayerConnection::processTimeRemainingFrame(Frame* frame){
    Logger::getLogger()->debug("Processing Get Time frame");
    Frame *time = createFrame(frame);
    time->setType(ft02_Time_Remaining);
    time->packInt(Game::getGame()->getTurnTimer()->secondsToEOT());
    if(time->getVersion() >= fv0_4){
        time->packInt(0); //player requested
        time->packInt(Game::getGame()->getTurnNumber());
        time->packString(Game::getGame()->getTurnName());
    }
    sendFrame(time);
}
