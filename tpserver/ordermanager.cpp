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
#include "frame.h"
#include "object.h"
#include "game.h"
#include "persistence.h"
#include "orderqueue.h"
#include "algorithms.h"

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

void OrderManager::describeOrder(uint32_t ordertype, Frame * f){
  if(prototype_store.find(ordertype) != prototype_store.end()){
    prototype_store[ordertype]->describeOrder(f);
  }else{
    f->createFailFrame(fec_NonExistant, "Order type does not exist");
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

void OrderManager::doGetOrderTypes(Frame* frame, Frame * of){
  uint32_t lseqkey = frame->unpackInt();
  if(lseqkey == UINT32_NEG_ONE){
    //start new seqkey
    lseqkey = seqkey;
  }

  uint32_t start = frame->unpackInt();
  uint32_t num = frame->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  if(lseqkey != seqkey){
    of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
    return;
  }

  IdModList modlist;
  for(PrototypeStore::iterator itcurr = prototype_store.begin();
      itcurr != prototype_store.end(); ++itcurr){
    Order* type = itcurr->second;
    if(fromtime == UINT64_NEG_ONE || type->getDescriptionModTime() > fromtime){
      modlist[itcurr->first] = type->getDescriptionModTime();
    }
  }

  if(start > modlist.size()){
    of->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }

  if(num > modlist.size() - start){
    num = modlist.size() - start;
  }

  if(num > MAX_ID_LIST_SIZE + ((of->getVersion() < fv0_4) ? 1 : 0)){
    of->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  of->setType(ft03_OrderTypes_List);
  of->packInt(lseqkey);
  of->packIdModList(modlist,num,start);
  if(of->getVersion() >= fv0_4){
    of->packInt64(fromtime);
  }

}

bool OrderManager::addOrderQueue(OrderQueue::Ptr oq){
  oq->setQueueId(orderqueue_next++);
  orderqueue_store[oq->getQueueId()] = oq;
  Game::getGame()->getPersistence()->saveOrderQueue(oq);
  return true;
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

void OrderManager::init(){
  orderqueue_next = Game::getGame()->getPersistence()->getMaxOrderQueueId() + 1;
}

