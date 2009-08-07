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
#include "playerview.h"
#include "game.h"
#include "persistence.h"
#include "ruleset.h"
#include "objectmanager.h"
#include "logging.h"
#include "boardmanager.h"
#include "board.h"
#include "message.h"
#include "designstore.h"
#include "design.h"
#include "algorithms.h"

#include "playermanager.h"

#include <boost/bind.hpp>

PlayerManager::PlayerManager(){
    nextid = 1;
}

PlayerManager::~PlayerManager(){

}

void PlayerManager::init(){
    Persistence* persist = Game::getGame()->getPersistence();
    nextid = persist->getMaxPlayerId() + 1;
    clear(persist->getPlayerIds());
}

Player::Ptr PlayerManager::createNewPlayer(const std::string &name, const std::string &pass){
  Player::Ptr rtn = findPlayer(name);

    if(rtn){
        //Player's name exists already
        rtn.reset();
    }else{
      //Player's name doesn't exist, create the new player.
      rtn.reset( new Player( nextid++, name, pass ) );

      // TODO: HACK - update ruleset!
      if(Game::getGame()->getRuleset()->onAddPlayer(rtn.get())){
        // player can be added
        
        //setup board and add to player
        Board::Ptr board = Game::getGame()->getBoardManager()->createNewBoard("Personal board", 
                "Messages from the System and personal notices board");
        rtn->setBoardId(board->getId());

        //add welcome message to player's board
        Message::Ptr msg( new Message() );
        msg->setSubject("Welcome");
        msg->setBody("Welcome to Thousand Parsec!\nThis server is running on tpserver-cpp.  Please report any problems and enjoy the game.");
        msg->addReference(rst_Special, rssv_System);
        msg->addReference(rst_Player, rtn->getID());
        board->addMessage(msg, -1);
        
        map[rtn->getID()] = (rtn);
        Game::getGame()->getPersistence()->savePlayer(rtn);
        
      // TODO: HACK - update ruleset!
        Game::getGame()->getRuleset()->onPlayerAdded(rtn.get());
        
        //tell the other players about it
        msg.reset( new Message() );
        msg->setSubject("New Player");
        msg->setBody("New player " + name + " has joined the game.");
        msg->addReference(rst_Special, rssv_System);
        msg->addReference(rst_Action_Player, rspav_Joined);
        msg->addReference(rst_Player, rtn->getID());
        
        for(Map::const_iterator itid = map.begin();
                itid != map.end(); ++itid){
          Player::Ptr op = itid->second;
            if(!op){
                op = Game::getGame()->getPersistence()->retrievePlayer(itid->first);
                map[itid->first] = op;
            }
            if(op && op != rtn){
                op->postToBoard(Message::Ptr( new Message( *msg ) ));
                //to update the difflist, etc
                op->getPlayerView()->doOnceATurn();
                Game::getGame()->getPersistence()->updatePlayer(op);
            }
        }

        rtn->getPlayerView()->doOnceATurn();

      }else{
          // player can not be added
          nextid--;
          rtn.reset();
      }
    
    }
    return rtn;
}

Player::Ptr PlayerManager::getPlayer(uint32_t id){
  Player::Ptr rtn;
    Map::iterator pl = map.find(id);
    if(pl != map.end()){
        rtn = (*pl).second;
    }else{
        //player does not exist
        return Player::Ptr();
    }
    if(!rtn){
        rtn = Game::getGame()->getPersistence()->retrievePlayer(id);
        if(rtn){
          map[id] = rtn;
        }
    }
    return rtn;
}

Player::Ptr PlayerManager::findPlayer(const std::string &name){
    Logger::getLogger()->debug("finding player");

    //look for current/known players
    Player::Ptr rtn;

    Map::iterator itcurr;

    for (itcurr = map.begin(); itcurr != map.end(); ++itcurr) {
      Player::Ptr p = (*itcurr).second;
        if(!p){
            p = Game::getGame()->getPersistence()->retrievePlayer(itcurr->first);
            itcurr->second = p;
        }
        if(p){
            std::string itname = p->getName();
            if (name == itname) {
                rtn = p;
                break;
            }
        }
    }

    return rtn;
}

void PlayerManager::updateAll(){
  std::for_each( map.begin(), map.end(), 
      boost::bind( &Persistence::updatePlayer, Game::getGame()->getPersistence(), 
        boost::bind( &Map::value_type::second, _1 )
      )
  );
}

void PlayerManager::updatePlayer(uint32_t id){
    Game::getGame()->getPersistence()->updatePlayer(map[id]);
}

uint32_t PlayerManager::getNumPlayers() const{
  return size();
}
