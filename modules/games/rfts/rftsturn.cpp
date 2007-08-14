/*  rftsturn
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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
#include <tpserver/objectdata.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>

#include "ownedobject.h"

#include "rftsturn.h"

namespace RFTS_ {

using std::set;

RftsTurn::RftsTurn() {

}

RftsTurn::~RftsTurn() {

}

void RftsTurn::doTurn() {

   Game* game = Game::getGame();
   OrderManager* ordermanager = game->getOrderManager();
   ObjectManager* objectmanager = game->getObjectManager();
   
   set<uint32_t> objectsIds = objectmanager->getAllIds();

   // currently just go through each obj and do each order
   // will be prioritized/sorted soon TODO

   for(set<uint32_t>::iterator i = objectsIds.begin();
         i != objectsIds.end(); ++i)
   {
      IGObject * currObj = objectmanager->getObject(*i);

      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(currObj->getObjectData()->getParameterByType(obpT_Order_Queue));
      OrderQueue* oq;
      if(oqop != NULL && 
         (oq = ordermanager->getOrderQueue(oqop->getQueueId())) != NULL)
         for(uint32_t j = 0; j < oq->getNumberOrders(); j++)
         {
            OwnedObject *orderedObj = dynamic_cast<OwnedObject*>(currObj->getObjectData());
            assert(orderedObj);

            Order* order = oq->getOrder(j, orderedObj->getOwner());
            if(order->doOrder(currObj))
            {
               oq->removeOrder(j, orderedObj->getOwner());
               j--; // list has been reordered
            }
            else
               oq->updateFirstOrder(); // check
         }
      currObj->touchModTime();
      objectmanager->doneWithObject(currObj->getID());
          
   }
   
   objectmanager->clearRemovedObjects();

   // to once a turn (right at the end)
   objectsIds = objectmanager->getAllIds();
   for(std::set<uint32_t>::iterator i = objectsIds.begin(); 
       i != objectsIds.end(); ++i)
   {
      IGObject * obj = objectmanager->getObject(*i);
      obj->getObjectData()->doOnceATurn(obj);
      objectmanager->doneWithObject(obj->getID());
   }

   objectmanager->clearRemovedObjects();

}

}
