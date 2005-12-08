/*  OrderManager for managing Order objects
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include "ordermanager.h"

OrderManager::OrderManager(){
  nextType = 0;
  
  seqkey = 1;
    nextOrderId = 0;
}

OrderManager::~OrderManager(){
  // I should clear the prototypeStore
  for(std::map<int, Order*>::iterator itcurr = prototypeStore.begin(); itcurr != prototypeStore.end(); ++itcurr){
    delete itcurr->second;
  }

}

bool OrderManager::checkOrderType(int type){
  return (type >= 0 && type <= nextType - 1);
}

void OrderManager::describeOrder(int ordertype, Frame * f){
  Order* prototype = prototypeStore[ordertype];
  if(prototype != NULL){
    prototype->describeOrder(f);
  }else{
    f->createFailFrame(fec_NonExistant, "Order type does not exist");
  }
}

Order* OrderManager::createOrder(int ot){
  Order* prototype = prototypeStore[ot];
  if(prototype != NULL){
    return prototype->clone();
  }
  return NULL;
}

void OrderManager::addOrderType(Order* prototype){
  prototype->setType(nextType);
  prototypeStore[nextType++] = prototype;
  seqkey++;
}

void OrderManager::doGetOrderTypes(Frame* frame, Frame * of){
   unsigned int lseqkey = frame->unpackInt();
  if(lseqkey == 0xffffffff){
    //start new seqkey
    lseqkey = seqkey;
  }

  unsigned int start = frame->unpackInt();
  unsigned int num = frame->unpackInt();

  if(lseqkey != seqkey){
    of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
    return;
  }

  unsigned int num_remain;
  if(num == 0xffffffff || start + num > prototypeStore.size()){
    num = prototypeStore.size() - start;
    num_remain = 0;
  }else{
    num_remain = prototypeStore.size() - start - num;
  }

  of->setType(ft03_ObjectIds_List);
  of->packInt(lseqkey);
  of->packInt(num_remain);
  of->packInt(num);
  std::map<int, Order*>::iterator itcurr = prototypeStore.begin();
  advance(itcurr, start);
  for(unsigned int i = 0; i < num; i++){
    of->packInt(itcurr->first);
    of->packInt64(0LL); //TODO mod time
    ++itcurr;
  }

}

bool OrderManager::addOrder(Order* ord, IGObject* obj, uint32_t pos, uint32_t playerid){
    if (obj->getObjectData()->checkAllowedOrder(ord->getType(), playerid)) {
        if(nextOrderId == 0){
            nextOrderId = Game::getGame()->getPersistence()->getMaxOrderId();
        }
        uint32_t orderid = nextOrderId++;
        ordercache[orderid] = ord;
        std::list<uint32_t> oolist = objectorders[obj->getID()];
        if(oolist.empty()){
            oolist = Game::getGame()->getPersistence()->retrieveOrderList(obj->getID());
            objectorders[obj->getID()] = oolist;
        }
        if (pos == 0xffffffff) {
            oolist.push_back(orderid);
        } else {
            std::list<uint32_t>::iterator inspos = oolist.begin();
            advance(inspos, pos);
            oolist.insert(inspos, orderid);
        }
        objectorders[obj->getID()] = oolist;
        Game::getGame()->getPersistence()->saveOrder(orderid, ord);
        Game::getGame()->getPersistence()->saveOrderList(obj->getID(), oolist);
        obj->setNumOrders(oolist.size());
        return true;
    }

    return false;

}

bool OrderManager::removeOrder(IGObject* obj, uint32_t pos, uint32_t playerid){
    std::list<uint32_t> oolist = objectorders[obj->getID()];
    if(oolist.empty()){
        oolist = Game::getGame()->getPersistence()->retrieveOrderList(obj->getID());
        objectorders[obj->getID()] = oolist;
    }
    if (pos >= oolist.size()) {
        return false;
    }

    std::list<uint32_t>::iterator itpos = oolist.begin();
    advance(itpos, pos);
    uint32_t orderid = *itpos;
    Order* ord = ordercache[orderid];
    if(ord == NULL){
        ord = Game::getGame()->getPersistence()->retrieveOrder(orderid);
        ordercache[orderid] = ord;
    }
    
    if(ord != NULL){
        if(obj->getObjectData()->checkAllowedOrder(ord->getType(), playerid)){
            delete ord;
            oolist.erase(itpos);
            objectorders[obj->getID()] = oolist;
            Game::getGame()->getPersistence()->removeOrder(orderid);
            Game::getGame()->getPersistence()->saveOrderList(obj->getID(), oolist);
            obj->setNumOrders(oolist.size());
            return true;
        }
        return false;
    }
    return false;
}

Order* OrderManager::getOrder(IGObject* obj, uint32_t pos, uint32_t playerid){
    std::list<uint32_t> oolist = objectorders[obj->getID()];
    if(oolist.empty()){
        oolist = Game::getGame()->getPersistence()->retrieveOrderList(obj->getID());
        objectorders[obj->getID()] = oolist;
    }
    if (pos >= oolist.size()) {
        return NULL;
    }

    std::list<uint32_t>::iterator itpos = oolist.begin();
    advance(itpos, pos);
    uint32_t orderid = *itpos;
    Order* ord = ordercache[orderid];
    if(ord == NULL){
        ord = Game::getGame()->getPersistence()->retrieveOrder(orderid);
        ordercache[orderid] = ord;
    }
    return ord;
}

Order * OrderManager::getFirstOrder(IGObject* obj){
    std::list<uint32_t> oolist = objectorders[obj->getID()];
    if(oolist.empty()){
        oolist = Game::getGame()->getPersistence()->retrieveOrderList(obj->getID());
        objectorders[obj->getID()] = oolist;
    }
    if(oolist.empty()){
        return NULL;
    }
    uint32_t orderid = oolist.front();
    Order* ord = ordercache[orderid];
    if(ord == NULL){
        ord = Game::getGame()->getPersistence()->retrieveOrder(orderid);
        ordercache[orderid] = ord;
    }
    return ord;
}

void OrderManager::removeFirstOrder(IGObject* obj){
    std::list<uint32_t> oolist = objectorders[obj->getID()];
    if(oolist.empty()){
        oolist = Game::getGame()->getPersistence()->retrieveOrderList(obj->getID());
        objectorders[obj->getID()] = oolist;
    }
    uint32_t orderid = oolist.front();
    oolist.pop_front();
    objectorders[obj->getID()] = oolist;
    Game::getGame()->getPersistence()->removeOrder(orderid);
    Game::getGame()->getPersistence()->saveOrderList(obj->getID(), oolist);
    obj->setNumOrders(oolist.size());
}

void OrderManager::updateFirstOrder(IGObject* obj){
    std::list<uint32_t> oolist = objectorders[obj->getID()];
    if(oolist.empty()){
        oolist = Game::getGame()->getPersistence()->retrieveOrderList(obj->getID());
        objectorders[obj->getID()] = oolist;
    }
    uint32_t orderid = oolist.front();
    Order* ord = ordercache[orderid];
    Game::getGame()->getPersistence()->updateOrder(orderid, ord);
    obj->touchModTime();
}

std::set<uint32_t> OrderManager::getObjectsWithOrders(){
    std::set<uint32_t> set;
    for(std::map<uint32_t, std::list<uint32_t> >::iterator itcurr = objectorders.begin();
            itcurr != objectorders.end(); ++itcurr){
        std::list<uint32_t> oolist = itcurr->second;
        if(oolist.empty()){
            oolist = Game::getGame()->getPersistence()->retrieveOrderList(itcurr->first);
            itcurr->second = oolist;
        }
        if(!oolist.empty()){
            set.insert(itcurr->first);
        }
    }
    return set;
}

void OrderManager::removeAllOrders(uint32_t objectid){
    std::list<uint32_t> oolist = objectorders[objectid];
    if(oolist.empty()){
        oolist = Game::getGame()->getPersistence()->retrieveOrderList(objectid);
        objectorders[objectid] = oolist;
    }
    std::list<uint32_t>::iterator itpos = oolist.begin();
    
    for(std::list<uint32_t>::iterator itpos = oolist.begin(); itpos != oolist.end(); ++itpos){
        uint32_t orderid = *itpos;
        Order* ord = ordercache[orderid];
        if(ord == NULL){
            delete ord;
        }
        Game::getGame()->getPersistence()->removeOrder(orderid);

    }
    
    //clear orderlist/slots
    oolist.clear();
    Game::getGame()->getPersistence()->saveOrderList(objectid, oolist);
    objectorders.erase(objectid);

}

void OrderManager::init(){
    std::set<uint32_t> oolist = Game::getGame()->getPersistence()->retrieveObjectsWithOrders();
    for(std::set<uint32_t>::iterator itcurr = oolist.begin(); itcurr != oolist.end(); ++itcurr){
        objectorders[*itcurr] = std::list<uint32_t>();
    }
}
