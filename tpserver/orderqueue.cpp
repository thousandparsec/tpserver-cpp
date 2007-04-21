/*  OrderQueue for managing Order objects
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include "order.h"
#include "frame.h"
#include "object.h"
#include "objectdata.h"
#include "game.h"
#include "persistence.h"

#include "orderqueue.h"

OrderQueue::OrderQueue(){
  queueid = 0;
  nextOrderId = 1;
}

OrderQueue::~OrderQueue(){
}

void OrderQueue::setQueueId(uint32_t id){
  queueid = id;
}

uint32_t OrderQueue::getQueueId() const{
  return queueid;
}


void OrderQueue::addOwner(uint32_t playerid){
  owner.insert(playerid);
}

void OrderQueue::removeOwner(uint32_t playerid){
  owner.erase(playerid);
}

std::set<uint32_t> OrderQueue::getOwner() const{
  return owner;
}

bool OrderQueue::checkOrderType(uint32_t type, uint32_t playerid) const{
  if(owner.find(playerid) != owner.end()){
    return allowedtypes.find(type) != allowedtypes.end();
  }
  return false;
}

std::set<uint32_t> OrderQueue::getAllowedOrderTypes() const{
  return allowedtypes;
}

void OrderQueue::addAllowedOrderType(uint32_t type){
  allowedtypes.insert(type);
}

void OrderQueue::removeAllowedOrderType(uint32_t type){
  allowedtypes.erase(type);
}

uint32_t OrderQueue::getNumberOrders() const{
  return orderlist.size();
}

bool OrderQueue::addOrder(Order* ord, uint32_t pos, uint32_t playerid){
  if (checkOrderType(ord->getType(), playerid)) {

    uint32_t orderid = nextOrderId++;
    ordercache[orderid] = ord;

    if (pos == 0xffffffff) {
      orderlist.push_back(orderid);
    } else {
      std::list<uint32_t>::iterator inspos = orderlist.begin();
      advance(inspos, pos);
      orderlist.insert(inspos, orderid);
    }
    Game::getGame()->getPersistence()->saveOrder(queueid, orderid, ord);
    Game::getGame()->getPersistence()->saveOrderQueue(this);
    return true;
  }

  return false;

}

Result OrderQueue::removeOrder(uint32_t pos, uint32_t playerid){
  if(owner.find(playerid) != owner.end()){
    if (pos >= orderlist.size()) {
      return Failure("Order slot to remove is passed end of order slots.");
    }

    std::list<uint32_t>::iterator itpos = orderlist.begin();
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
      Game::getGame()->getPersistence()->removeOrder(queueid, orderid);
      Game::getGame()->getPersistence()->saveOrderQueue(this);
      return Success();
    }
    return Failure("No such Order");
  }
  return Failure("Not allowed");
}

Order* OrderQueue::getOrder(uint32_t pos, uint32_t playerid){
  if(owner.find(playerid) != owner.end()){
    if (pos >= orderlist.size()) {
      return NULL;
    }

    std::list<uint32_t>::iterator itpos = orderlist.begin();
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
  Game::getGame()->getPersistence()->saveOrderQueue(this);
}

void OrderQueue::updateFirstOrder(){
  uint32_t orderid = orderlist.front();
  Order* ord = ordercache[orderid];
  Game::getGame()->getPersistence()->updateOrder(queueid, orderid, ord);
}


void OrderQueue::removeAllOrders(){
  std::list<uint32_t>::iterator itpos = orderlist.begin();
  
  for(std::list<uint32_t>::iterator itpos = orderlist.begin(); itpos != orderlist.end(); ++itpos){
    uint32_t orderid = *itpos;
    Order* ord = ordercache[orderid];
    if(ord == NULL){
      delete ord;
    }
    Game::getGame()->getPersistence()->removeOrder(queueid, orderid);

  }
  
  //clear orderlist/slots
  orderlist.clear();

}

void OrderQueue::setNextOrderId(uint32_t next){
  nextOrderId = next;
}
