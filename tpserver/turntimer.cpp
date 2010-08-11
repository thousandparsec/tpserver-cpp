/*  TurnTimer class
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

#include "game.h"
#include "logging.h"
#include "net.h"
#include "playermanager.h"
#include "player.h"
#include "settings.h"
#include "asynctimeremaining.h"

#include "turntimer.h"

TurnTimer::TurnTimer(): finishedPlayers(), numdead(0){
}

TurnTimer::~TurnTimer(){
  if(timer){
      timer->invalidate();
      timer.reset();
  }
  finishedPlayers.clear();
}
uint32_t TurnTimer::secondsToEOT() const{
    if(timer != NULL){
        return timer->getExpireTime() - time(NULL);
    }
    return UINT32_NEG_ONE;
}

uint32_t TurnTimer::getTurnLength() const{
    Settings* settings = Settings::getSettings();
  
    uint32_t len = atoi(settings->get("turn_length").c_str());
    if(len == 0){
        len = UINT32_NEG_ONE;
    }
    return len;
}


void TurnTimer::playerFinishedTurn(uint32_t playerid){
  Logger::getLogger()->info("Player %d finished turn (%d other players finished).", playerid, finishedPlayers.size());

  if(Game::getGame()->getPlayerManager()->getPlayer(playerid)->isAlive()){
    bool added = finishedPlayers.insert(playerid).second;
    onPlayerFinishedTurn();

    if (added) {
      uint32_t len = secondsToEOT();
      sendTimeRemaining(len, 2);
    }
  }
}

void TurnTimer::manuallyRunEndOfTurn(){
  // send frame to all connections that the end of turn has started
  sendTimeRemaining(0, 5);
  
  doEndOfTurn();
}

void TurnTimer::setNumberDeadPlayers(uint32_t ndp){
  numdead = ndp;
}


uint32_t TurnTimer::getNumActivePlayers() const{
    return Game::getGame()->getPlayerManager()->getNumPlayers() - numdead;
}

uint32_t TurnTimer::getNumDonePlayers() const{
    return finishedPlayers.size();
}

void TurnTimer::onPlayerFinishedTurn(){
    //Do nothing
}

void TurnTimer::timerExpiredStartEOT(){
    sendTimeRemaining(0, 5);
  
    doEndOfTurn();
}

void TurnTimer::allDoneStartEOT(){
    sendTimeRemaining(0, 3);
  
    doEndOfTurn();
}


void TurnTimer::timerStarted(){
    uint32_t len = secondsToEOT();
    sendTimeRemaining(len, 1);
}

void TurnTimer::thresholdFinishedNewTimer(){
    uint32_t len = secondsToEOT();
    sendTimeRemaining(len, 4);
}

void TurnTimer::advancedWarningOfTimer(){
    uint32_t len = secondsToEOT();
    sendTimeRemaining(len, 2);
}

std::set<playerid_t> TurnTimer::getPlayers(){
    PlayerManager::Ptr pm = Game::getGame()->getPlayerManager();
  
    std::set<playerid_t> waiting;
    std::set<playerid_t> all = pm->getAllIds();
    for(std::set<playerid_t>::iterator itplayer = all.begin(); itplayer != all.end(); ++itplayer){
        if(finishedPlayers.find(*itplayer) == finishedPlayers.end()){
            Player::Ptr player = pm->getPlayer(*itplayer);
            if(player->isAlive()){
                waiting.insert(*itplayer);
            }
        }
    }
    return waiting;
} 

void TurnTimer::sendTimeRemaining(uint32_t remaining, uint32_t reason){
    AsyncFrame * aframe = new AsyncTimeRemaining(remaining, reason, getPlayers());
    Network::getNetwork()->sendToAll(aframe);
}

void TurnTimer::doEndOfTurn(){
    Game::getGame()->doEndOfTurn();
    finishedPlayers.clear();
    resetTimer();
    Network::getNetwork()->doneEOT();
}

void TurnTimer::timerFinished(){
    //EOT
    timerExpiredStartEOT();
}
