/*  RiskTurn object
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/playermanager.h>
#include <tpserver/object.h>
#include <tpserver/order.h>
#include <tpserver/objectbehaviour.h>
#include <tpserver/objectview.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/message.h>
#include <stdint.h>
#include <tpserver/settings.h>
#include <tpserver/logging.h>

#include <boost/format.hpp>
#include "risk.h"
#include "riskturn.h"
#include "ownedobject.h"

namespace RiskRuleset{

using std::set;
using std::string;
using std::map;
using boost::format;

RiskTurn::RiskTurn() : TurnProcess() {

} 
    
RiskTurn::~RiskTurn(){
    
}

void RiskTurn::doTurn(){
   Game* game = Game::getGame();
   ObjectManager* objM = game->getObjectManager();
   PlayerManager* pm = game->getPlayerManager();
   set<uint32_t> objectsIds = objM->getAllIds();

   processOrdersOfGivenType("Colonize");
   processOrdersOfGivenType("Reinforce");
   processOrdersOfGivenType("Move");
   processOrdersOfGivenType();
   
   calculateReinforcements(); 
   
   setPlayerVisibleObjects();

   // do once a turn (right at the end)
   objectsIds = objM->getAllIds();

   for(std::set<uint32_t>::iterator i = objectsIds.begin(); i != objectsIds.end(); ++i)
   {
      IGObject * obj = objM->getObject(*i);
      obj->getObjectBehaviour()->doOnceATurn();
      objM->doneWithObject(obj->getID());
   }

   objM->clearRemovedObjects();
   
   Player* winner = getWinner();
   if(winner != NULL)
   {
      set<uint32_t> players = pm->getAllIds();

      string body;
      Message *gameOver = new Message();
      gameOver->setSubject("Game over!");
      
      if( game->getTurnNumber() ==
            static_cast<unsigned>(strtol(Settings::getSettings()->get("game_length").c_str(), NULL, 10)) )
         body = "Game length elapsed, winner is: ";
      else
         body = "Universal domination by: ";
         
      body += winner->getName();
      gameOver->setBody(body);

      for(set<uint32_t>::iterator i = players.begin(); i != players.end(); ++i)
         pm->getPlayer(*i)->postToBoard(gameOver);
   }
    
} //RiskTurn::RiskTurn() : TurnProcess()

void RiskTurn::calculateReinforcements() {
   Logger::getLogger()->debug("Starting RiskTurn::calculateReinforcements");
   
   Game* game = Game::getGame();
   ObjectManager* om = game->getObjectManager();
   PlayerManager* pm = game->getPlayerManager();
   Risk* risk = dynamic_cast<Risk*>(game->getRuleset());

   set<uint32_t> objectsIds = om->getAllIds();
   map<uint32_t,uint32_t> planets_owned; // ownerID=>num_owned
   
   uint32_t rfc_rate = atoi(Settings::getSettings()->get("risk_rfc_rate").c_str() );
   uint32_t rfc_number = atoi(Settings::getSettings()->get("risk_rfc_number").c_str() );
   Logger::getLogger()->debug("Got reinforcement rate and number, they are %d and %d",rfc_rate, rfc_number);
   
   //For each object in the universe tabulate how many planets each player owns
   for(set<uint32_t>::iterator i = objectsIds.begin(); i != objectsIds.end(); ++i) {
      OwnedObject* currObj = dynamic_cast<OwnedObject*>(om->getObject(*i)->getObjectBehaviour());
      if (currObj != NULL) {//if the cast was ok
         uint32_t owner = currObj->getOwner();
         if (owner != 0) {
            planets_owned[owner] += 1; //add 1 to the number of planets owned for that player
            Logger::getLogger()->debug("Got owned planet, owner is %d",owner);
         }
      }
   }   

   //for each player in the map, calculate their reinforcements and add them to the existing rfc.
   for(map<uint32_t,uint32_t>::iterator i = planets_owned.begin(); i != planets_owned.end(); ++i) {
      uint32_t number = rfc_number*(i->second/rfc_rate);
      uint32_t current = risk->getPlayerReinforcements(i->first);
      risk->setPlayerReinforcements(i->first,current+number);
      Logger::getLogger()->debug("Reinforcements set for Player %d, current total is now: %d",i->first,risk->getPlayerReinforcements(i->first));
      
      //Produce owner reinforcements message
      Message* message = new Message();
      assert(message);
      string subject = "You have received new reinforcements";
      format body("You have gained %1% for a new total of %2% units.");
      body % number; body % risk->getPlayerReinforcements(i->first);
      message->setSubject(subject);
      message->setBody(body.str());
      pm->getPlayer(i->first)->postToBoard(message);
   }
   
   calculateBonusReinforcements();
}

void RiskTurn::calculateBonusReinforcements() {
   //LATER: Give out bonus reinforcements for owning whole constellations
}


/** processOrdersOfGivenType
* This function iterates over all objects the objM holds and process only orders of a given type 
* that are in the front of the queue.
* I.E. if the type passed is P and the queue for an object is P->P->Q->P
*   only the first two P will be procesed
*NOTE: If no type is specified then all orders in queue will be processed
**/
void RiskTurn::processOrdersOfGivenType(string type) {
   Logger::getLogger()->debug("Now processing orders of type: \"%s\"",type.c_str());
   Game* game = Game::getGame();
   OrderManager* ordM = game->getOrderManager();
   ObjectManager* objM = game->getObjectManager();
      
   //Get all objects from object manager
   set<uint32_t> objectsIds = objM->getAllIds();
   
   //Iterate over every object
   for(set<uint32_t>::iterator i = objectsIds.begin(); i != objectsIds.end(); ++i)
   {
      IGObject * currObj = objM->getObject(*i);
      
      //Get order queue from object
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(currObj->getParameterByType(obpT_Order_Queue));
      OrderQueue* oq;

      if(oqop != NULL && (oq = ordM->getOrderQueue(oqop->getQueueId())) != NULL)
      {
         //Iterate over all orders
         for (uint32_t j = 0; j < oq->getNumberOrders(); j++)
         {
            //Taken from RFTS
            OwnedObject *orderedObj = dynamic_cast<OwnedObject*>(currObj->getObjectBehaviour());
            assert(orderedObj);

            Order* order = oq->getOrder(j, orderedObj->getOwner());
            //if order is of type asked for then process it, also process if no type is specified
            if(order->getName() == type || type == "")
            {

               if(order->doOrder(currObj))
               {
                  oq->removeOrder(j, orderedObj->getOwner());
                  j--; // list has been reordered
               }
               else
                  oq->updateFirstOrder(); // CHECK
            }
            else
            {
               j = oq->getNumberOrders() + 1;   //force the loop to exit, we have encountered an order not of given type
            }
         }
         currObj->touchModTime();
      }
      objM->doneWithObject(currObj->getID());
   }
}

void RiskTurn::setPlayerVisibleObjects() {
   
   PlayerManager *pm = Game::getGame()->getPlayerManager();
   
   set<uint32_t> players = pm->getAllIds();
   
   for(set<uint32_t>::const_iterator i = players.begin(); i != players.end(); i++) {
      PlayerView* playerview = pm->getPlayer(*i)->getPlayerView();
      std::set<uint32_t> objids = Game::getGame()->getObjectManager()->getAllIds();
      for(std::set<uint32_t>::iterator itcurr = objids.begin(); itcurr != objids.end();
         ++itcurr){
      ObjectView* obv = new ObjectView();
      obv->setObjectId(*itcurr);
      obv->setCompletelyVisible(true);
      playerview->addVisibleObject(obv);
      }
   }
}

Player* RiskTurn::getWinner() {
   Logger::getLogger()->debug("Looking for a winner");
   Game* game = Game::getGame();
   PlayerManager* pm = game->getPlayerManager();
   ObjectManager* objM = game->getObjectManager();
   
   Player *winner = NULL;

   //Get all objects from object manager
   set<uint32_t> objectsIds = objM->getAllIds();
   set<uint32_t> owners;
   uint32_t owner;
   
   //Iterate over every object
   for(set<uint32_t>::iterator i = objectsIds.begin(); i != objectsIds.end(); ++i)
   {  
      IGObject * currObj = objM->getObject(*i);
      OwnedObject *ownedObj = dynamic_cast<OwnedObject*>(currObj->getObjectBehaviour());

      if ( ownedObj != NULL) { //if the object IS owned
         owner = ownedObj->getOwner(); 
         if (owner != 0) {          //if the object is owned by a player
            owners.insert(owner);   //Add the object's owner to the set
            
            format message ("Adding owner #%1% to owners");
            message % owner;
            Logger::getLogger()->debug(message.str().c_str());
         }
      }
   }
   
   if ( owners.size() == 1 )  {      //If there is only one owner in the list
      winner = pm->getPlayer(*(owners.begin()));//Get the player and assign them as the winner
      format message ("The winner is %1%");
      message % *(owners.begin());
      Logger::getLogger()->debug(message.str().c_str());
   }
   return winner;
}
} //namespace RiskRuleset
