/*  PlayerManager
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include "player.h"
#include "game.h"
#include "persistence.h"
#include "ruleset.h"
#include "objectmanager.h"
#include "logging.h"

#include "playermanager.h"

PlayerManager::PlayerManager(){
    nextid = 1;
}

PlayerManager::~PlayerManager(){

}

void PlayerManager::init(){

}

Player* PlayerManager::createNewPlayer(const std::string &name, const std::string &pass){
    Player *rtn = NULL;
    rtn = new Player();
    rtn->setId(nextid++);
    rtn->setName(name.c_str());
    rtn->setPass(pass.c_str());

    if(Game::getGame()->getRuleset()->onAddPlayer(rtn)){
        // player can be added
        players[rtn->getID()] = (rtn);
        
        Game::getGame()->getRuleset()->onPlayerAdded(rtn);
        
        //HACK
        rtn->setVisibleObjects(Game::getGame()->getObjectManager()->getAllIds());

    }else{
        // player can not be added
        delete rtn;
        rtn = NULL;
        nextid--;
    }
    return rtn;
}

Player* PlayerManager::getPlayer(uint32_t id){
    Player* rtn = NULL;
    std::map<unsigned int, Player*>::iterator pl = players.find(id);
    if(pl != players.end()){
        rtn = (*pl).second;
    }
    return rtn;
}

Player* PlayerManager::findPlayer(const std::string &name, const std::string &pass){
    Logger::getLogger()->debug("finding player");

    //look for current/known players
    Player *rtn = NULL;

    // hack HACK!!
    if (name == "guest" && pass == "guest")
        return rtn;
    // end of hack HACK!!

    std::map<unsigned int, Player*>::iterator itcurr;

    for (itcurr = players.begin(); itcurr != players.end(); ++itcurr) {
        std::string itname = (*itcurr).second->getName();
        if (name == itname) {
            rtn = (*itcurr).second;
            break;
        }
    }
    
    //name not found, throw an exception to know that we can create a new player
    if(rtn == NULL)
        throw std::exception();
    
    std::string itpass = rtn->getPass();
    if (pass != itpass) {
        rtn = NULL;
    }

    return rtn;
}

void PlayerManager::updateAll(){

}

void PlayerManager::updatePlayer(uint32_t id){
    
}

std::set<uint32_t> PlayerManager::getAllIds(){
    std::set<unsigned int> vis;
    for(std::map<unsigned int, Player*>::const_iterator itid = players.begin();
        itid != players.end(); ++itid){
        vis.insert(itid->first);
    }
    return vis;
}
