

#include "order.h"
#include "frame.h"

// needed to setup inbuilt orders
#include "nop.h"
#include "move.h"
#include "build.h"
#include "colonise.h"
#include "splitfleet.h"
#include "mergefleet.h"

#include "ordermanager.h"

OrderManager::OrderManager(){
  nextType = 1000;

  prototypeStore[odT_Nop] = new Nop();
  prototypeStore[odT_Move] = new Move();
  prototypeStore[odT_Build] = new Build();
  prototypeStore[odT_Colonise] = new Colonise();
  prototypeStore[odT_Fleet_Split] = new SplitFleet();
  prototypeStore[odT_Fleet_Merge] = new MergeFleet();

}

OrderManager::~OrderManager(){
  // I should clear the prototypeStore

}

bool OrderManager::checkOrderType(int type){
  return ((type > odT_Invalid && type < odT_Max) || (type >= 1000 && type <= nextType - 1));
}

void OrderManager::describeOrder(int ordertype, Frame * f){
  Order* prototype = prototypeStore[ordertype];
  if(prototype != NULL){
    prototype->describeOrder(f);
  }else{
    f->createFailFrame(fec_PermUnavailable, "Order type does not exist");
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
  prototypeStore[nextType++] = prototype;
}
