/*  PlayersFinishedTurnTimer class
 *
 *  Copyright (C) 2009  Lee Begg and the Thousand Parsec Project
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

#include "settings.h"
#include "net.h"
#include "timercallback.h"
#include "protocol.h"
#include "game.h"
#include "playermanager.h"

#include "playersfinishedturntimer.h"

PlayersFinishedTurnTimer::PlayersFinishedTurnTimer() : TurnTimer() {
    
}

void PlayersFinishedTurnTimer::resetTimer(){
    if(timer != NULL){
        timer->invalidate();
        timer.reset();
    }
    uint32_t len = atoi(Settings::getSettings()->get("turn_length").c_str());
    
    if(len != 0){
        timer.reset( new TimerCallback( boost::bind( &PlayersFinishedTurnTimer::timerFinished, this ), len) );
        Network::getNetwork()->addTimer(timer);
    
        //Alert players
        timerStarted();
    }
}

void PlayersFinishedTurnTimer::onPlayerFinishedTurn(){
    uint32_t minPlayers = 2;
    Settings* settings = Settings::getSettings();
    if(settings->get("turn_players_min").length() > 0){
        minPlayers = atoi(settings->get("turn_players_min").c_str());
    }
    bool requireActivePlayer = (settings->get("turn_require_active_players") == "yes");
    if(Game::getGame()->getPlayerManager()->getNumPlayers() >= minPlayers){
        if((getNumActivePlayers() > 1 || !requireActivePlayer) && getNumActivePlayers() == getNumDonePlayers()){
            allDoneStartEOT();
        }
    }
}
