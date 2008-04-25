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

#include "ordermanager.h"

OrderManager::OrderManager(){
  nextType = 0;
  nextOrderQueueId = 1;
  seqkey = 1;
}

OrderManager::~OrderManager(){
  // I should clear the prototypeStore
  for(std::map<uint32_t, Order*>::iterator itcurr = prototypeStore.begin(); itcurr != prototypeStore.end(); ++itcurr){
    delete itcurr->second;
  }

}

bool OrderManager::checkOrderType(uint32_t type){
  return (type >= 0 && type <= nextType - 1);
}

void OrderManager::describeOrder(uint32_t ordertype, Frame * f){
  if(prototypeStore.find(ordertype) != prototypeStore.end()){
    prototypeStore[ordertype]->describeOrder(f);
  }else{
    f->createFailFrame(fec_NonExistant, "Order type does not exist");
  }
}

Order* OrderManager::createOrder(uint32_t ot){
  if(prototypeStore.find(ot) != prototypeStore.end()){
    return prototypeStore[ot]->clone();
  }
  return NULL;
}

void OrderManager::addOrderType(Order* prototype){
  prototype->setType(nextType);
  prototypeStore[nextType++] = prototype;
  typeNames[prototype->getName()] = prototype->getType();
  seqkey++;
}

uint32_t OrderManager::getOrderTypeByName(const std::string& name){
  if(typeNames.find(name) == typeNames.end()){
    return UINT32_NEG_ONE;
  }else
    return typeNames[name];
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

  std::map<uint32_t, uint64_t> modlist;
  for(std::map<uint32_t, Order*>::iterator itcurr = prototypeStore.begin();
      itcurr != prototypeStore.end(); ++itcurr){
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
  of->packInt(modlist.size() - start - num);
  of->packInt(num);
  std::map<uint32_t, uint64_t>::iterator itcurr = modlist.begin();
  advance(itcurr, start);
  for(uint32_t i = 0; i < num; i++){
    of->packInt(itcurr->first);
    of->packInt64(itcurr->second);
    ++itcurr;
  }

  if(of->getVersion() >= fv0_4){
    of->packInt64(fromtime);
  }
  
}

bool OrderManager::addOrderQueue(OrderQueue* oq){
  oq->setQueueId(nextOrderQueueId++);
  orderqueues[oq->getQueueId()] = oq;
  Game::getGame()->getPersistence()->saveOrderQueue(oq);
  return true;
}

void OrderManager::updateOrderQueue(uint32_t oqid){
  OrderQueue *oq = orderqueues[oqid];
  Game::getGame()->getPersistence()->updateOrderQueue(oq);
}

bool OrderManager::removeOrderQueue(uint32_t oqid){
  OrderQueue *oq = orderqueues[oqid];
  if(oq == NULL){
    oq = Game::getGame()->getPersistence()->retrieveOrderQueue(oqid);
  }
  if(oq != NULL){
    Game::getGame()->getPersistence()->removeOrderQueue(oqid);
    delete oq;
    orderqueues.erase(oqid);
    return true;
  }else{
    return false;
  }
}

OrderQueue* OrderManager::getOrderQueue(uint32_t oqid){
  OrderQueue *oq = orderqueues[oqid];
  if(oq == NULL){
    oq = Game::getGame()->getPersistence()->retrieveOrderQueue(oqid);
    orderqueues[oqid] = oq;
  }
  return oq;
}

void OrderManager::init(){
  nextOrderQueueId = Game::getGame()->getPersistence()->getMaxOrderQueueId() + 1;
}
