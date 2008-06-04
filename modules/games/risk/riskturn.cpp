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
#include "tpserver/objectbehaviour.h"
#include <tpserver/order.h>
#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/message.h>
#include <stdint.h>

#include "riskturn.h"
#include "ownedobject.h"

namespace RiskRuleset{

using std::set;
using std::string;

RiskTurn::RiskTurn() : TurnProcess() {

} 
    
RiskTurn::~RiskTurn(){
    
}

void RiskTurn::doTurn(){
   Game* game = Game::getGame();
   ObjectManager* objM = game->getObjectManager();
   PlayerManager *pm = game->getPlayerManager();
   set<uint32_t> objectsIds = objM->getAllIds();


   //TODO: Calculate new reinforcements for players, add to their total.
   //Send message to player about updated total

   // currently just go through each obj and do each order
   // will be prioritized/sorted soon 
   //TODO: Process orders like so:
      // Process every objects colonize orders - Check for conflicts, pick a "winner"
      // Process every objects Reinforce orders
      // Process every objects Move orders
      // Process every objects remaining orders
   //TODO: BIG: figure out some way to keep track of a players availible reinforcements
  

   processOrdersOfGivenType(objM,"Colonize");
   processOrdersOfGivenType(objM,"Reinforce");
   processOrdersOfGivenType(objM,"Move");
   processOrdersOfGivenType(objM);

   //objM->clearRemovedObjects(); //CHECK: no objects are ever removed in Risk
   
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
    
} //RiskTurn::RiskTurn() : TurnProcess()

//ASK: Should I be producing more detailed function documentation? or only in cases were it is a little weird
/** processOrdersOfGivenType
* This function iterates over all objects the objM holds and process only orders of a given type 
* that are in the front of the queue.
* I.E. if the type passed is P and the queue for an object is P->P->Q->P
*   only the first two P will be procesed
*NOTE: If no type is specified then all orders in queue will be processed
*/
void RiskTurn::processOrdersOfGivenType(ObjectManager* objM, string type) {
   Game* game = Game::getGame();
   OrderManager* ordM = game->getOrderManager();
   
   //Get all objects frobjM object manager
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
            //if order is of type asked for then process it
            if(type != "" && order->getName() == type)
            {
               //CHECK: taken directly from RFTS code
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
   //CHECK: If I need to reset any views. Its my belief I don't need to, since no objects are ever created.
}

Player* RiskTurn::getWinner() {
   //TODO: Check for winner here
   Player *winner = NULL;  //So it compiles
   return winner;
}
} //namespace RiskRuleset
