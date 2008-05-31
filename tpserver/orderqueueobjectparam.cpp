/*  OrderQueueObjectParam baseclass
 *
 *  Copyright (C) 2007 Lee Begg and the Thousand Parsec Project
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

#include "frame.h"
#include "game.h"
#include "ordermanager.h"
#include "orderqueue.h"

#include "orderqueueobjectparam.h"

OrderQueueObjectParam::OrderQueueObjectParam() : ObjectParameter(), queueid(0){
  type = obpT_Order_Queue;
}

OrderQueueObjectParam::~OrderQueueObjectParam(){

}


void OrderQueueObjectParam::packObjectFrame(Frame * f, uint32_t playerid){
  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
  if(orderqueue->isOwner(playerid)){
    f->packInt(queueid);
    f->packInt(orderqueue->getNumberOrders());
    // order types
    std::set<uint32_t> allowedtypes = orderqueue->getAllowedOrderTypes();
    f->packInt(allowedtypes.size());
    for(std::set<uint32_t>::iterator itcurr = allowedtypes.begin(); itcurr != allowedtypes.end();
        ++itcurr){
      f->packInt(*itcurr);
    }
    
  }else{
    f->packInt(0);
    f->packInt(0);
    f->packInt(0);
  }
}

bool OrderQueueObjectParam::unpackModifyObjectFrame(Frame *f, unsigned int playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(12))
    return false;
  //queueid = f->unpackInt();
  //numorders = f->unpackInt();
  f->setUnpackOffset(f->getUnpackOffset() + 8);
  uint32_t listsize = f->unpackInt();
  if(!f->isEnoughRemaining(4 * listsize))
    return false;
  f->setUnpackOffset(f->getUnpackOffset() + 4 * listsize);
  return true;
}

void OrderQueueObjectParam::signalRemoval(){
  Game::getGame()->getOrderManager()->getOrderQueue(queueid)->removeAllOrders();
}

ObjectParameter* OrderQueueObjectParam::clone() const{
  return new OrderQueueObjectParam();
}

void OrderQueueObjectParam::setQueueId(uint32_t nqi){
  queueid = nqi;
}

uint32_t OrderQueueObjectParam::getQueueId() const{
  return queueid;
}

uint32_t OrderQueueObjectParam::getNumOrders() const{
  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
  return orderqueue->getNumberOrders();
}

std::set<uint32_t> OrderQueueObjectParam::getAllowedOrders() const{
  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
  return orderqueue->getAllowedOrderTypes();
}

void OrderQueueObjectParam::setAllowedOrders(std::set<uint32_t> ao){
  OrderQueue* orderqueue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
  orderqueue->setAllowedOrderTypes(ao);
  Game::getGame()->getOrderManager()->updateOrderQueue(queueid);
}
