/*  ThresholdTurnTimer class
 *
 *  Copyright (C) 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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
#include "logging.h"
#include "net.h"
#include "settings.h"
#include "settingscallback.h"
#include "protocol.h"

#include "thresholdturntimer.h"

ThresholdTurnTimer::ThresholdTurnTimer(): TurnTimer(), timer(NULL), overthreshold(false){
  Settings::getSettings()->setCallback("turn_player_threshold", SettingsCallback(this, &ThresholdTurnTimer::thresholdChanged));
}

ThresholdTurnTimer::~ThresholdTurnTimer(){
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
  }
  Settings::getSettings()->removeCallback("turn_player_threshold");
}

void ThresholdTurnTimer::onPlayerFinishedTurn(){
    if(getNumDonePlayers() > 1 && getNumDonePlayers() == getNumActivePlayers()){
        //All players finished (and two players or more)
        allDoneStartEOT();
    }else if(!overthreshold && isOverThreshold()){
        Logger::getLogger()->info("Threshold of players finished, setting over threshold turn length.");
        updateTimer();
        overthreshold = true;
    }
}


uint32_t ThresholdTurnTimer::secondsToEOT() const{
  if(timer != NULL){
    return timer->getExpireTime() - time(NULL);
  }
  return UINT32_NEG_ONE;
}

uint32_t ThresholdTurnTimer::getTurnLength() const{
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
      return UINT32_NEG_ONE;
    }
    return (len_over_thres > len_under_thres) ? len_over_thres : len_under_thres;
  }
}

void ThresholdTurnTimer::resetTimer(){
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
      allDoneStartEOT();
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
      // But do send an asynchTimeRemaining to alert players that the EOT finished
      timerStarted();

      return;
    }
  }

  timer = new TimerCallback(this, &ThresholdTurnTimer::timerFinished, increment);
  Network::getNetwork()->addTimer(*timer);
  
  // send frame to all connections that the end of turn has started
  timerStarted();
  
}

void ThresholdTurnTimer::updateTimer(){
  Settings * settings = Settings::getSettings();
  uint32_t increment = atoi(settings->get("turn_length_over_threshold").c_str());
  
  if(increment == 0 && atoi(settings->get("turn_player_threshold").c_str()) > 0){
    // if no increment and threshold is greater than 0, the do the end of turn straight away
    allDoneStartEOT();
    
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
  
  timer = new TimerCallback(this, &ThresholdTurnTimer::timerFinished, increment);
  Network::getNetwork()->addTimer(*timer);
  
  // send frame to all connections that the end of turn has started
  timerExpiredStartEOT();
}

void ThresholdTurnTimer::timerFinished(){
    //Turn timer expired, do end of turn
    timerExpiredStartEOT();
}


bool ThresholdTurnTimer::isOverThreshold(){
  uint32_t threshold = atoi(Settings::getSettings()->get("turn_player_threshold").c_str());
  uint32_t numPlayers = getNumActivePlayers();
  return ((numPlayers == 0) ? false : getNumDonePlayers() * 100 >= threshold * numPlayers);
}

void ThresholdTurnTimer::thresholdChanged(const std::string& key, const std::string& val){
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
