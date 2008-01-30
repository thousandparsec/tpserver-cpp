/*  rftsturn
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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

#include <algorithm>
#include <cassert>

#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/order.h>
#include <tpserver/object.h>
#include <tpserver/objectbehaviour.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/settings.h>
#include <tpserver/message.h>

#include "ownedobject.h"
#include "playerinfo.h"

#include "rftsturn.h"

namespace RFTS_ {

using std::set;
using std::string;

RftsTurn::RftsTurn() {

}

RftsTurn::~RftsTurn() {

}

void RftsTurn::doTurn() {

   Game* game = Game::getGame();
   OrderManager* ordermanager = game->getOrderManager();
   ObjectManager* objectmanager = game->getObjectManager();
   PlayerManager *pm = game->getPlayerManager();

   set<uint32_t> objectsIds = objectmanager->getAllIds();

   // currently just go through each obj and do each order
   // will be prioritized/sorted soon TODO

   for(set<uint32_t>::iterator i = objectsIds.begin();
         i != objectsIds.end(); ++i)
   {
      IGObject * currObj = objectmanager->getObject(*i);

      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(currObj->getParameterByType(obpT_Order_Queue));
      OrderQueue* oq;
      if(oqop != NULL && 
         (oq = ordermanager->getOrderQueue(oqop->getQueueId())) != NULL)
      {
         for(uint32_t j = 0; j < oq->getNumberOrders(); j++)
         {
            OwnedObject *orderedObj = dynamic_cast<OwnedObject*>(currObj->getObjectBehaviour());
            assert(orderedObj);

            Order* order = oq->getOrder(j, orderedObj->getOwner());
            if(order->doOrder(currObj))
            {
               oq->removeOrder(j, orderedObj->getOwner());
               j--; // list has been reordered
            }
            else
               oq->updateFirstOrder(); // CHECK
         }
         currObj->touchModTime();
      }
      objectmanager->doneWithObject(currObj->getID());
          
   }
   
   objectmanager->clearRemovedObjects();

   // re-explore new area
   setPlayerVisibleObjects();

   // to once a turn (right at the end)
   objectsIds = objectmanager->getAllIds();
   for(std::set<uint32_t>::iterator i = objectsIds.begin(); 
       i != objectsIds.end(); ++i)
   {
      IGObject * obj = objectmanager->getObject(*i);
      obj->getObjectBehaviour()->doOnceATurn();
      objectmanager->doneWithObject(obj->getID());
   }

   objectmanager->clearRemovedObjects();
   
   // update in case fleets were destroyed in combat
   setPlayerVisibleObjects();

   set<uint32_t> players = pm->getAllIds();
   for(set<uint32_t>::iterator i = players.begin(); i != players.end(); ++i)
      PlayerInfo::getPlayerInfo(*i).clearPdbUpgrade();

   Player* winner = getWinner();
   if(winner != NULL)
   {
      string body;
      Message *gameOver = new Message();
      gameOver->setSubject("Game over!");
      
      if( game->getTurnNumber() ==
            static_cast<unsigned>(strtol(Settings::getSettings()->get("game_length").c_str(), NULL, 10)) )
         body = "Game length elapsed, winner is: ";
      else
         body = "Overwhelming victory by: ";
         
      body += winner->getName();
      gameOver->setBody(PlayerInfo::appAllVictoryPoints(body));

      for(set<uint32_t>::iterator i = players.begin(); i != players.end(); ++i)
         pm->getPlayer(*i)->postToBoard(gameOver);
   }
}

void RftsTurn::setPlayerVisibleObjects() {
   PlayerManager *pm = Game::getGame()->getPlayerManager();
   
   set<uint32_t> gameObjects = Game::getGame()->getObjectManager()->getAllIds();
   set<uint32_t> players = pm->getAllIds();
   
   for(set<uint32_t>::const_iterator i = players.begin(); i != players.end(); i++)
   {
      set<uint32_t> ownedObjects;
      findOwnedObjects(*i, gameObjects, ownedObjects);
      
      Player *player = pm->getPlayer(*i);
      setVisibleObjects(player, ownedObjects);
   }
}

Player* RftsTurn::getWinner() {

   Game *game = Game::getGame();
   
   unsigned gameLength = strtol(Settings::getSettings()->get("game_length").c_str(), NULL, 10);

   uint32_t highestScore = 0, nextHighest = 0;
   Player *winner = NULL;
   
   
   // check for overwhelming victory (only if we're reasonablly started)
   if(game->getTurnNumber() > gameLength / 4.)
   {
      set<uint32_t> players = game->getPlayerManager()->getAllIds();
      for(set<uint32_t>::iterator i = players.begin(); i!= players.end(); ++i)
      {
         uint32_t vp = PlayerInfo::getPlayerInfo(*i).getVictoryPoints();
         if( vp > highestScore)
         {
            nextHighest = highestScore;
            highestScore = vp;
            winner = game->getPlayerManager()->getPlayer(*i); // save winner
         }
      }

      // clear 'winner' if: NOT in an overwhelming victory state
      // or                game is NOT over
      if( highestScore < (nextHighest * 2.5) ||
          game->getTurnNumber() != gameLength )
         winner = NULL;
   }

   return winner;
}


// helpers

void setVisibleObjects(Player *player, const set<uint32_t>& ownedObjects) {
   ObjectManager *om = Game::getGame()->getObjectManager();
   
   IGObject *universe = om->getObject(0);
   PlayerView *pv = player->getPlayerView();

   // add universe and star systems
   pv->setVisibleObjects(universe->getContainedObjects());
   pv->addVisibleObject(universe->getID());

   for(set<uint32_t>::const_iterator i = ownedObjects.begin(); i != ownedObjects.end(); ++i)
   {
      IGObject *obj = om->getObject(*i);
      exploreStarSys(obj);
   }
      
   
}

void findOwnedObjects(uint32_t playerId, set<uint32_t>& gameObjects, set<uint32_t>& ownedObjects) {
   ObjectManager *om = Game::getGame()->getObjectManager();
   
   for(set<uint32_t>::const_iterator i = gameObjects.begin(); i != gameObjects.end(); ++i)
   {
      OwnedObject *obj = dynamic_cast<OwnedObject*>(om->getObject(*i)->getObjectBehaviour());
      if(obj != NULL && obj->getOwner() == playerId)
      {
         ownedObjects.insert(*i);
         gameObjects.erase(*i);
      }
   }
}

}
