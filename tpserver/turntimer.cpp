/*  TurnTimer class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include <ctime>

#include "timercallback.h"
#include "game.h"
#include "logging.h"
#include "net.h"
#include "settings.h"
#include "settingscallback.h"
#include "playermanager.h"
#include "frame.h"
#include "asynctimeremaining.h"

#include "turntimer.h"

TurnTimer::TurnTimer(): timer(NULL), finishedPlayers(), overthreshold(false){
  Settings::getSettings()->setCallback("turn_player_threshold", SettingsCallback(this, &TurnTimer::thresholdChanged));
}

TurnTimer::~TurnTimer(){
  finishedPlayers.clear();
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
  }
  Settings::getSettings()->removeCallback("turn_player_threshold");
}

void TurnTimer::playerFinishedTurn(uint32_t playerid){
  finishedPlayers.insert(playerid);
  if(!overthreshold && isOverThreshold()){
    Logger::getLogger()->info("Threshold of players finished, setting over threshold turn length.");
    updateTimer();
    overthreshold = true;
  }
}


uint32_t TurnTimer::secondsToEOT() const{
  if(timer != NULL){
    return timer->getExpireTime() - time(NULL);
  }
  return 0xffffffff;
}

uint32_t TurnTimer::getTurnLength() const{
  // It's hard to put an exact length on a turn
  // Using max of turn_length_over_threshold and turn_length_under_threshold
  // unless turn_player_threshold is 0
  Settings* settings = Settings::getSettings();
  
  uint32_t len_over_thres = atoi(settings->get("turn_length_over_threshold").c_str());
  uint32_t len_under_thres = atoi(settings->get("turn_length_under_threshold").c_str());
  if(atoi(settings->get("turn_player_threshold").c_str()) == 0){
    return len_over_thres;
  }else{
    if(len_under_thres == 0){
      return 0xffffff;
    }
    return (len_over_thres > len_under_thres) ? len_over_thres : len_under_thres;
  }
}

void TurnTimer::resetTimer(){
  Settings* settings = Settings::getSettings();
  
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
    timer = NULL;
  }
  
  uint64_t increment;
  if(isOverThreshold()){
    increment = atoi(settings->get("turn_length_over_threshold").c_str());
    if(increment == 0 && atoi(settings->get("turn_player_threshold").c_str()) > 0){
      // if no increment and threshold is greater than 0, the do the end of turn straight away
      thresholdDoneAndStartEOT();
      return;
    }
    if(increment <= 0){
      Logger::getLogger()->warning("turn_length_over_threshold too small for turn_player_threshold = 0, increasing to 60 seconds");
      increment = 60;
    }
  }else{
    increment = atoi(settings->get("turn_length_under_threshold").c_str());
    if(increment == 0){
      // wait forever, don't set timer
      return;
    }
  }

  timer = new TimerCallback(this, &TurnTimer::timerFinished, increment);
  Network::getNetwork()->addTimer(*timer);
  
  // send frame to all connections that the end of turn has started
  AsyncFrame * aframe = new AsyncTimeRemaining(secondsToEOT(), 1); //timer started
  Network::getNetwork()->sendToAll(aframe);
  
}

void TurnTimer::manuallyRunEndOfTurn(){
  // send frame to all connections that the end of turn has started
  AsyncFrame * aframe = new AsyncTimeRemaining(0, 5); //EOT started
  Network::getNetwork()->sendToAll(aframe);
  
  Game::getGame()->doEndOfTurn();
  finishedPlayers.clear();
  overthreshold = false;
  resetTimer();
  
  Network::getNetwork()->doneEOT();
}

void TurnTimer::updateTimer(){
  Settings * settings = Settings::getSettings();
  uint32_t increment = atoi(settings->get("turn_length_over_threshold").c_str());
  
  if(increment == 0 && atoi(settings->get("turn_player_threshold").c_str()) > 0){
    // if no increment and threshold is greater than 0, the do the end of turn straight away
    thresholdDoneAndStartEOT();
    return;
  }
  
  // if 0 seconds or less, set to 60 seconds
  if(increment <= 0){
    Logger::getLogger()->warning("Turn length (over threshold) too short, was %d seconds, now 60 seconds", increment);
    increment = 60;
  }
  
  if(timer != NULL){
    if(timer->getExpireTime() <= increment + time(NULL)){
      // current timer is shorter, keep using it
      return;
    }
    timer->setValid(false);
    delete timer;
  }
  
  timer = new TimerCallback(this, &TurnTimer::timerFinished, increment);
  Network::getNetwork()->addTimer(*timer);
  
  // send frame to all connections that the end of turn has started
  AsyncFrame * aframe = new AsyncTimeRemaining(increment, 4); //threshold finished, timer started
  Network::getNetwork()->sendToAll(aframe);
}

void TurnTimer::timerFinished(){
  // send frame to all connections that the end of turn has started
  AsyncFrame * aframe = new AsyncTimeRemaining(0, 5); //EOT started
  Network::getNetwork()->sendToAll(aframe);
  
  Game::getGame()->doEndOfTurn();
  finishedPlayers.clear();
  overthreshold = false;
  resetTimer();
  Network::getNetwork()->doneEOT();
}

void TurnTimer::thresholdDoneAndStartEOT(){
  // send frame to all connections that the end of turn has started
  AsyncFrame * aframe = new AsyncTimeRemaining(0, 3); //threshold finished and eot started
  Network::getNetwork()->sendToAll(aframe);
  
  Game::getGame()->doEndOfTurn();
  finishedPlayers.clear();
  overthreshold = false;
  resetTimer();
  Network::getNetwork()->doneEOT();
}

bool TurnTimer::isOverThreshold(){
  uint32_t threshold = atoi(Settings::getSettings()->get("turn_player_threshold").c_str());
  uint32_t numPlayers = Game::getGame()->getPlayerManager()->getNumPlayers();
  return (numPlayers == 0) ? false : finishedPlayers.size() * 100 >= threshold * numPlayers;
}

void TurnTimer::thresholdChanged(const std::string& key, const std::string& val){
  if(isOverThreshold()){
    if(!overthreshold){
      updateTimer();
      overthreshold = true;
    }
  }else{
    if(overthreshold){
      resetTimer();
      overthreshold = false;
    }
  }
}
