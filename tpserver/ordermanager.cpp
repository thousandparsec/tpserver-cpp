/*  OrderManager for managing Order objects
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include "object.h"
#include "game.h"
#include "persistence.h"
#include "orderqueue.h"
#include "algorithms.h"
#include "common.h"
#include "refsys.h"

#include "ordermanager.h"

OrderManager::OrderManager() : prototype_next(0), orderqueue_next(1), seqkey(1) {
}

OrderManager::~OrderManager(){
  // I should clear the prototype_store
  delete_map_all( prototype_store );
}

bool OrderManager::checkOrderType(uint32_t type){
  return (type >= 0 && type <= prototype_next - 1);
}

void OrderManager::describeOrder(uint32_t ordertype, OutputFrame::Ptr f){
  if(prototype_store.find(ordertype) != prototype_store.end()){
    prototype_store[ordertype]->describeOrder(f);
  }else{
    RefList reflist;
    reflist.push_back(RefTypeAndId(rst_OrderType, ordertype));
    throw FrameException(fec_NonExistant, "Order type does not exist", reflist);
  }
}

Order* OrderManager::createOrder(uint32_t ot){
  if(prototype_store.find(ot) != prototype_store.end()){
    return prototype_store[ot]->clone();
  }
  return NULL;
}

void OrderManager::addOrderType(Order* prototype){
  prototype->setType(prototype_next);
  prototype_store[prototype_next++] = prototype;
  typename_map[prototype->getName()] = prototype->getType();
  seqkey++;
}

uint32_t OrderManager::getOrderTypeByName(const std::string& name){
  if(typename_map.find(name) == typename_map.end()){
    return UINT32_NEG_ONE;
  }else
    return typename_map[name];
}

uint32_t OrderManager::addOrderQueue( uint32_t objectid, uint32_t ownerid )
{
  uint32_t order_queue_id = orderqueue_next++;
  OrderQueue::Ptr oq( new OrderQueue( order_queue_id, objectid, ownerid ) );
  orderqueue_store[order_queue_id] = oq;
  Game::getGame()->getPersistence()->saveOrderQueue(oq);
  return order_queue_id;
}

void OrderManager::updateOrderQueue(uint32_t oqid){
  OrderQueue::Ptr oq = orderqueue_store[oqid];
  Game::getGame()->getPersistence()->updateOrderQueue(oq);
}

bool OrderManager::removeOrderQueue(uint32_t oqid){
  OrderQueue::Ptr oq = orderqueue_store[oqid];
  if(oq == NULL){
    oq = Game::getGame()->getPersistence()->retrieveOrderQueue(oqid);
  }
  if(oq != NULL){
    Game::getGame()->getPersistence()->removeOrderQueue(oqid);
    orderqueue_store.erase(oqid);
    return true;
  }else{
    return false;
  }
}

OrderQueue::Ptr OrderManager::getOrderQueue(uint32_t oqid){
  OrderQueue::Ptr oq = orderqueue_store[oqid];
  if(oq == NULL){
    oq = Game::getGame()->getPersistence()->retrieveOrderQueue(oqid);
    orderqueue_store[oqid] = oq;
  }
  return oq;
}

IdModList OrderManager::getModList( uint64_t fromtime ) const {
  IdModList modlist;
  for(PrototypeStore::const_iterator itcurr = prototype_store.begin();
      itcurr != prototype_store.end(); ++itcurr){
    const Order* type = itcurr->second;
    if(fromtime == UINT64_NEG_ONE || type->getDescriptionModTime() > fromtime){
      modlist[itcurr->first] = type->getDescriptionModTime();
    }
  }
  return modlist;
}

void OrderManager::init(){
  orderqueue_next = Game::getGame()->getPersistence()->getMaxOrderQueueId() + 1;
}

