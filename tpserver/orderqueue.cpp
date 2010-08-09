/*  OrderQueue for managing Order objects
 *
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

#include <time.h>

#include "order.h"
#include "object.h"
#include "game.h"
#include "persistence.h"
#include "objectmanager.h"
#include "algorithms.h"

#include "orderqueue.h"

OrderQueue::OrderQueue( uint32_t nid, uint32_t nobjectid, uint32_t nownerid ) : queueid(nid), objectid(nobjectid), active(true), repeating(false) {
  addOwner( nownerid );
  nextOrderId = 1;
}

OrderQueue::~OrderQueue(){
}

uint32_t OrderQueue::getQueueId() const{
  return queueid;
}


void OrderQueue::addOwner(uint32_t playerid){
  owner.insert(playerid);
  touchModTime();
}

void OrderQueue::removeOwner(uint32_t playerid){
  owner.erase(playerid);
  touchModTime();
}

bool OrderQueue::isOwner(uint32_t playerid) const{
  return owner.find(playerid) != owner.end();
}

IdSet OrderQueue::getOwner() const{
  return owner;
}

void OrderQueue::setOwners(IdSet no){
  owner = no;
}

uint32_t OrderQueue::getObjectId() const{
  return objectid;
}

bool OrderQueue::checkOrderType(uint32_t type, uint32_t playerid) const{
  if(isOwner(playerid)){
    return allowedtypes.find(type) != allowedtypes.end();
  }
  return false;
}

IdSet OrderQueue::getAllowedOrderTypes() const{
  return allowedtypes;
}

void OrderQueue::addAllowedOrderType(uint32_t type){
  allowedtypes.insert(type);
  touchModTime();
}

void OrderQueue::removeAllowedOrderType(uint32_t type){
  allowedtypes.erase(type);
  touchModTime();
}

void OrderQueue::setAllowedOrderTypes(const IdSet& ao){
  allowedtypes = ao;
  touchModTime();
}

uint32_t OrderQueue::getNumberOrders() const{
  return orderlist.size();
}

bool OrderQueue::addOrder(Order* ord, uint32_t pos, uint32_t playerid){
  if (checkOrderType(ord->getType(), playerid)) {

    uint32_t orderid = nextOrderId++;
    ordercache[orderid] = ord;
    ord->setOrderQueueId(queueid);

    if (pos == UINT32_NEG_ONE) {
      orderlist.push_back(orderid);
    } else {
      IdList::iterator inspos = orderlist.begin();
      advance(inspos, pos);
      orderlist.insert(inspos, orderid);
    }
    Game::getGame()->getPersistence()->saveOrder(queueid, orderid, ord);
    touchModTime();
    Game::getGame()->getPersistence()->updateOrderQueue(shared_from_this());

    return true;
  }

  return false;

}

bool OrderQueue::removeOrder(uint32_t pos, uint32_t playerid){
  if(isOwner(playerid)){
    if (pos >= orderlist.size()) {
      return false;
    }

    IdList::iterator itpos = orderlist.begin();
    advance(itpos, pos);
    uint32_t orderid = *itpos;
    Order* ord = ordercache[orderid];
    if(ord == NULL){
      ord = Game::getGame()->getPersistence()->retrieveOrder(queueid, orderid);
      ordercache[orderid] = ord;
    }
    
    if(ord != NULL){
      delete ord;
      orderlist.erase(itpos);
      ordercache.erase(orderid);
      Game::getGame()->getPersistence()->removeOrder(queueid, orderid);
      touchModTime();
      Game::getGame()->getPersistence()->updateOrderQueue(shared_from_this());
      return true;
    }
    return false;
  }
  return false;
}

Order* OrderQueue::getOrder(uint32_t pos, uint32_t playerid){
  if(isOwner(playerid)){
    if (pos >= orderlist.size()) {
      return NULL;
    }

    IdList::iterator itpos = orderlist.begin();
    advance(itpos, pos);
    uint32_t orderid = *itpos;
    Order* ord = ordercache[orderid];
    if(ord == NULL){
      ord = Game::getGame()->getPersistence()->retrieveOrder(queueid, orderid);
      ordercache[orderid] = ord;
    }
    return ord;
  }
  return NULL;
}

Order * OrderQueue::getFirstOrder(){
  if(orderlist.empty()){
    return NULL;
  }
  uint32_t orderid = orderlist.front();
  Order* ord = ordercache[orderid];
  if(ord == NULL){
    ord = Game::getGame()->getPersistence()->retrieveOrder(queueid, orderid);
    ordercache[orderid] = ord;
  }
  return ord;
}

void OrderQueue::removeFirstOrder(){
  uint32_t orderid = orderlist.front();
  orderlist.pop_front();
  Game::getGame()->getPersistence()->removeOrder(queueid, orderid);
  touchModTime();
  Game::getGame()->getPersistence()->updateOrderQueue(shared_from_this());
}

void OrderQueue::updateFirstOrder(){
  uint32_t orderid = orderlist.front();
  Order* ord = ordercache[orderid];
  Game::getGame()->getPersistence()->updateOrder(queueid, orderid, ord);
  touchModTime();
  Game::getGame()->getPersistence()->updateOrderQueue(shared_from_this());
}

void OrderQueue::setActive(bool a){
  active = a;
  touchModTime();
}

void OrderQueue::setRepeating(bool r){
  repeating = r;
  touchModTime();
}

bool OrderQueue::isActive() const{
  return active;
}

bool OrderQueue::isRepeating() const{
  return repeating;
}

void OrderQueue::touchModTime(){
  Modifiable::touchModTime();
  if(objectid != 0){
    IGObject::Ptr robj = Game::getGame()->getObjectManager()->getObject(objectid);
    if(robj != NULL){
      robj->touchModTime();
      Game::getGame()->getObjectManager()->doneWithObject(objectid);
    }
  }
}

void OrderQueue::removeAllOrders(){
  IdList::iterator itpos = orderlist.begin();
  
  for(IdList::iterator itpos = orderlist.begin(); itpos != orderlist.end(); ++itpos){
    uint32_t orderid = *itpos;
    Order* ord = ordercache[orderid];
    if(ord == NULL){
      delete ord;
    }
    Game::getGame()->getPersistence()->removeOrder(queueid, orderid);

  }
  
  //clear orderlist/slots
  orderlist.clear();
  touchModTime();
  Game::getGame()->getPersistence()->updateOrderQueue(shared_from_this());
}

void OrderQueue::pack(OutputFrame::Ptr frame) const {
  frame->setType(ft04_OrderQueue);
  frame->packInt(queueid);
  frame->packInt64(getModTime());
  frame->packInt(-1);
  frame->packInt(orderlist.size());
  frame->packInt(allowedtypes.size());
  for(IdSet::iterator itat = allowedtypes.begin(); itat != allowedtypes.end(); ++itat){
    frame->packInt(*itat);
  }
}

void OrderQueue::setNextOrderId(uint32_t next){
  nextOrderId = next;
}

void OrderQueue::setOrderSlots(IdList nos){
  orderlist = nos;
}

IdList OrderQueue::getOrderSlots() const{
  return orderlist;
}
