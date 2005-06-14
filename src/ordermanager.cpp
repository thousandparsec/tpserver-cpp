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


#include "ordermanager.h"

OrderManager::OrderManager(){
  nextType = 0;
  
  seqkey = 1;
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
