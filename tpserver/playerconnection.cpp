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
#include "game.h"
#include "player.h"
#include "playermanager.h"
#include "settings.h"
#include "playeragent.h"
#include "turntimer.h"

#include "playerconnection.h"

PlayerConnection::PlayerConnection(int fd) 
  : TcpConnection(fd, PLAYER)
{
  lastpingtime = time(NULL);
}

PlayerConnection::~PlayerConnection(){
}


void PlayerConnection::processLogin(){
  InputFrame::Ptr recvframe( new InputFrame(version,paddingfilter) );
  if (readFrame(recvframe)) {
    try {
      if(recvframe->getType() == ft02_Login){
        processLoginFrame(recvframe);
      }else if(version >= fv0_3 && recvframe->getType() == ft03_Features_Get){
        processGetFeaturesFrame(recvframe);
      }else if(recvframe->getType() == ft02_Time_Remaining_Get){
        processTimeRemainingFrame(recvframe);
      }else if(version >= fv0_3 && recvframe->getType() == ft03_Account){
        processAccountFrame(recvframe);
      }else if(version >= fv0_4 && recvframe->getType() == ft04_GameInfo_Get){
        processGetGameInfoFrame(recvframe);
      }else if(version >= fv0_4 && recvframe->getType() == ft04_Filters_Set){
        processSetFilters(recvframe);
      }else{
        WARNING("PlayerConnection : In connected state but did not receive login, get features or get get time remaining, received frame type %d", recvframe->getType());
        throw FrameException( fec_FrameError, "Wrong type of frame in this state, wanted login, account or get features");
      }
    } catch ( FrameException& exception ) {
      // This might be overkill later, but now let's log it
      DEBUG( "PlayerConnection caught FrameException : %s", exception.what() );
      sendFail( recvframe, exception.getErrorCode(), exception.getErrorMessage() );
    }
  }
}

void PlayerConnection::processAccountFrame(InputFrame::Ptr frame)
{
  if(Settings::getSettings()->get("add_players") == "yes"){
    std::string username = frame->unpackString();
    std::string password = frame->unpackString();
    username = username.substr(0, username.find('@'));
    if (username.length() > 0 && password.length() > 0) {
      INFO("PlayerConnection : Creating new player");
      Player::Ptr player = Game::getGame()->getPlayerManager()->createNewPlayer(username, password);
      if(player != NULL){
        // also email address and comment strings
        player->setEmail(frame->unpackString());
        player->setComment(frame->unpackString());
        sendOK(frame,"Account created.");
        INFO("PlayerConnection : Account created ok for %s", username.c_str());
        playeragent.reset( new PlayerAgent(boost::dynamic_pointer_cast<PlayerConnection>(shared_from_this()),player) );
      }else{
        INFO("PlayerConnection : Bad username or password in account creation");
        throw FrameException( fec_FrameError, "Account creation Error - bad username or password");	// TODO - should be a const or enum, Login error
      }
    }else{
      DEBUG("PlayerConnection : username or password == NULL in account frame");
      throw FrameException( fec_FrameError, "Account Error - no username or password");	// TODO - should be a const or enum, Login error
    }
  }else{
    INFO("PlayerConnection : Account creation disabled, not creating account");
    throw FrameException( fec_PermissionDenied, "Account creation Disabled, talk to game admin");
  }
}

void PlayerConnection::processLoginFrame(InputFrame::Ptr frame)
{
  std::string username, password;

  // Get authentication data
  if ( getAuth( frame, username, password ) ) {
    Player::Ptr player = Game::getGame()->getPlayerManager()->findPlayer(username);

    // if player exists
    if (player) {
      if(player->getPass() != password){
        // password doesn't match, fail
        player.reset();
      }
    } else {
      //Player's name doesn't exist
      if( Settings::getSettings()->get("autoadd_players") == "yes" &&
          Settings::getSettings()->get("add_players") == "yes") {
        INFO("PlayerConnection : Creating new player automatically");
        player = Game::getGame()->getPlayerManager()->createNewPlayer(username, password);
        if( !player ) {
          throw FrameException( fec_PermissionDenied, "Cannot create new player");
        }
      }
    }
    if (player) {
      sendOK(frame, "Welcome");
      Logger::getLogger()->info("Login ok by %s", username.c_str());
      playeragent.reset( new PlayerAgent(boost::dynamic_pointer_cast<PlayerConnection>(shared_from_this()),player) );
      status = READY;
    } else {
      INFO("PlayerConnection : Bad username or password");
      throw FrameException( fec_FrameError, "Login Error - bad username or password");	// TODO - should be a const or enum, Login error
    }
  }
}

void PlayerConnection::processNormalFrame()
{
  InputFrame::Ptr frame( new InputFrame(version,paddingfilter) );
  if (readFrame(frame)) {
    try {
      if(version >= fv0_3 && frame->getType() == ft03_Features_Get){
        processGetFeaturesFrame(frame);
      }else if(version >= fv0_3 && frame->getType() == ft03_Ping){
        processPingFrame(frame);
      }else if(frame->getType() == ft02_Time_Remaining_Get){
        processTimeRemainingFrame(frame);
      }else if(version >= fv0_4 && frame->getType() == ft04_GameInfo_Get){
        processGetGameInfoFrame(frame);
      }else if(version >= fv0_4 && frame->getType() == ft04_Filters_Set){
        processSetFilters(frame);
      }else{
        if(Game::getGame()->isStarted()){
          // should pass frame to player to do something with
          DEBUG("PlayerConnection : inGameFrame");
          playeragent->processIGFrame(frame);
        }else{
          throw FrameException( fec_TempUnavailable, "Game has not yet started, please wait..." );
        }
      }
    } catch ( FrameException& exception ) {
      // This might be overkill later, but now let's log it
      DEBUG( "PlayerConnection caught FrameException : %s", exception.what() );
      sendFail( frame, exception );
    }
  } else {
    DEBUG("PlayerConnection : noFrame :(");
    // client closed
  }
}

void PlayerConnection::processPingFrame(InputFrame::Ptr frame)
{
  DEBUG("PlayerConnection : Processing Ping frame");
  // check for the time of the last frame, ignore this if
  //  less than 60 seconds ago.
  if(lastpingtime < static_cast<uint64_t>(time(NULL)) - 60){
    lastpingtime = time(NULL);
    sendOK(frame, "Keep alive ok, hope you're still there");
    DEBUG("PlayerConnection : Did ping for client %d", sockfd);
  }else{
    WARNING("PlayerConnection : Client %d tried to ping within 60 seconds of the last ping", sockfd);
  }
}

void PlayerConnection::processGetFeaturesFrame(InputFrame::Ptr frame){
  DEBUG("PlayerConnection : Processing Get Features frame");
  OutputFrame::Ptr features = createFrame(frame);
  features->setType(ft03_Features);
  IdSet fids;
  fids.insert(fid_keep_alive);
  fids.insert(fid_serverside_property);
  if(frame->getVersion() >= fv0_4){
    fids.insert(fid_filter_stringpad);
  }
  if(Settings::getSettings()->get("add_players") == "yes"){
    fids.insert(fid_account_register);
  }

  features->packIdSet(fids);
  sendFrame(features);
}

void PlayerConnection::processGetGameInfoFrame(InputFrame::Ptr frame){
  DEBUG("PlayerConnection : Processing get GameInfo frame");
  OutputFrame::Ptr game = createFrame(frame);
  Game::getGame()->packGameInfoFrame(game);
  sendFrame(game);
}

void PlayerConnection::processSetFilters(InputFrame::Ptr frame){
  DEBUG("PlayerConnection : Processing set filters");
  IdSet filters_wanted = frame->unpackIdSet();

  IdSet filters_setup;
  if(filters_wanted.count(fid_filter_stringpad) != 0){
    filters_setup.insert(fid_filter_stringpad);
  }

  if(filters_wanted.size() != filters_setup.size()){
    throw FrameException( fec_PermUnavailable, "Not all filters specified are available");
  }else{
    sendOK(frame, "Filters ready, setting filters now");
    if(filters_setup.count(fid_filter_stringpad) != 0){
      paddingfilter = true;
    }
  }
}

void PlayerConnection::processTimeRemainingFrame(InputFrame::Ptr frame){
  DEBUG("PlayerConnection : Processing Get Time frame");
  OutputFrame::Ptr time = createFrame(frame);
  time->setType(ft02_Time_Remaining);
  time->packInt(Game::getGame()->getTurnTimer()->secondsToEOT());
  if(time->getVersion() >= fv0_4){
    time->packInt(0); //player requested
    time->packInt(Game::getGame()->getTurnNumber());
    time->packString(Game::getGame()->getTurnName());
  }
  sendFrame(time);
}

