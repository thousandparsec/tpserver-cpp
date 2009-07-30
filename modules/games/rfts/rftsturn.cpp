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
#include <tpserver/objecttypemanager.h>
#include <tpserver/order.h>
#include <tpserver/object.h>
#include <tpserver/objectbehaviour.h>
#include <tpserver/objectview.h>
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
      Message::Ptr gameOver( new Message() );
      gameOver->setSubject("Game over!");
      
      if( game->getTurnNumber() ==
            static_cast<unsigned>(strtol(Settings::getSettings()->get("game_length").c_str(), NULL, 10)) )
         body = "Game length elapsed, winner is: ";
      else
         body = "Overwhelming victory by: ";
         
      body += winner->getName();
      gameOver->setBody(PlayerInfo::appAllVictoryPoints(body));

      for(set<uint32_t>::iterator i = players.begin(); i != players.end(); ++i)
         pm->getPlayer(*i)->postToBoard( Message::Ptr( new Message(*gameOver) ));
   }
   
   int turn = game->getTurnNumber() % 3;
   if(turn == 0){
       game->setTurnName("Production, Construction, Movement");
   }else if(turn == 1){
       game->setTurnName("Construction, Movement");
   }else{
       game->setTurnName("Movement");
   }
}

void RftsTurn::setPlayerVisibleObjects() {
   PlayerManager *pm = Game::getGame()->getPlayerManager();
   
   set<uint32_t> gameObjects = Game::getGame()->getObjectManager()->getAllIds();
   set<uint32_t> players = pm->getAllIds();
   
   for(set<uint32_t>::const_iterator i = players.begin(); i != players.end(); i++)
   {
      Player *player = pm->getPlayer(*i);
      setVisibleObjects(player);
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

void setVisibleObjects(Player *player) {
   ObjectManager *om = Game::getGame()->getObjectManager();
   
   IGObject *universe = om->getObject(0);
   PlayerView::Ptr pv = player->getPlayerView();
   set<uint32_t> ownedObjects = pv->getOwnedObjects();

   // add universe and star systems
   ObjectView* obv = pv->getObjectView(universe->getID());
   if(obv == NULL){
      obv = new ObjectView();
      obv->setObjectId(universe->getID());
      obv->setCompletelyVisible(true);
      pv->addVisibleObject(obv);
   }
   
   uint32_t fleettype = Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Fleet");
   set<uint32_t> containedObjects = universe->getContainedObjects();
   for(set<uint32_t>::const_iterator i = containedObjects.begin(); i != containedObjects.end(); ++i){
     IGObject* object = om->getObject(*i);
     if(object->getType() != fleettype){
      obv = pv->getObjectView(*i);
      if(obv == NULL){
          obv = new ObjectView();
          obv->setObjectId(*i);
          obv->setCompletelyVisible(true);
          pv->addVisibleObject(obv);
      }
     }else{
       obv = pv->getObjectView(*i);
       if(obv != NULL && !obv->isGone()){
         obv->setGone(true);
         pv->updateObjectView(*i);
       }
     }
     om->doneWithObject(*i);
   }

   for(set<uint32_t>::const_iterator i = ownedObjects.begin(); i != ownedObjects.end(); ++i)
   {
      IGObject *obj = om->getObject(*i);
      if(obj->getType() != fleettype || obj->getParent() != 0){
        exploreStarSys(obj);
      }
      om->doneWithObject(*i);
   }
   
   set<uint32_t> visobjects = pv->getVisibleObjects();
   for(set<uint32_t>::const_iterator i = visobjects.begin(); i != visobjects.end(); ++i)
   {
      IGObject *obj = om->getObject(*i);
      obv = pv->getObjectView(*i);
      if(obj == NULL){
        if(!obv->isGone()){
          obv->setGone(true);
          pv->updateObjectView(*i);
        }
      }else if((!obv->isGone()) && obv->isCompletelyVisible() && obj->getModTime() > obv->getModTime()){
        obv->setModTime(obj->getModTime());
        pv->updateObjectView(*i);
      }
      if(obj != NULL){
        om->doneWithObject(*i);
      }
   }
   
}


}
