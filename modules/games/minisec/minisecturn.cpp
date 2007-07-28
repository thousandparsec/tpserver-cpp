/*  MinisecTurn object
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

#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/playermanager.h>
#include <tpserver/order.h>
#include <tpserver/object.h>
#include "planet.h"
#include "fleet.h"
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/objectdata.h>
#include <tpserver/objectparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/ordermanager.h>

#include "rspcombat.h"

#include "minisecturn.h"

MinisecTurn::MinisecTurn() : TurnProcess(), containerids(){
  
}

MinisecTurn::~MinisecTurn(){
  
}

void MinisecTurn::doTurn(){
  std::set<uint32_t>::iterator itcurr;
  
  Game* game = Game::getGame();
  OrderManager* ordermanager = game->getOrderManager();
  ObjectManager* objectmanager = game->getObjectManager();
  RSPCombat* combatstrategy = new RSPCombat();
  PlayerManager* playermanager = game->getPlayerManager();

  //sort by order type
  std::set<uint32_t> movers;
  std::set<uint32_t> otherorders;
  
  containerids.clear();
  std::set<uint32_t> possiblecombatants;
  
  std::set<uint32_t> objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    if(ob->getType() == planettype || ob->getType() == fleettype){
      possiblecombatants.insert(ob->getID());
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getObjectData()->getParameterByType(obpT_Order_Queue));
      if(oqop != NULL){
        OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
        if(orderqueue != NULL){
          Order * currOrder = orderqueue->getFirstOrder();
          if(currOrder != NULL){
            if(currOrder->getType() == ordermanager->getOrderTypeByName("Move")){
              movers.insert(ob->getID());
            }else{
              otherorders.insert(ob->getID());
            }
          }
        }
      }
    }
    if(ob->getContainerType() >= 1){
      containerids.insert(ob->getID());
    }
    objectmanager->doneWithObject(ob->getID());
  }
  
  // do move
  for(itcurr = movers.begin(); itcurr != movers.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getObjectData()->getParameterByType(obpT_Order_Queue));
    OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
    Order * currOrder = orderqueue->getFirstOrder();
    if(currOrder->doOrder(ob)){
      orderqueue->removeFirstOrder();
    }else{
      orderqueue->updateFirstOrder();
    }
    
    objectmanager->doneWithObject(ob->getID());
  }
  
  // do combat
  
  for(itcurr = possiblecombatants.begin(); itcurr != possiblecombatants.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    uint32_t playerid1;
    Vector3d pos1;
    uint32_t size1;
    if(ob->getType() == planettype){
      Planet* planet = (Planet*)(ob->getObjectData());
      playerid1 = planet->getOwner();
      pos1 = planet->getPosition();
      size1 = planet->getSize();
    }else{
      Fleet* fleet = (Fleet*)(ob->getObjectData());
      playerid1 = fleet->getOwner();
      pos1 = fleet->getPosition();
      size1 = fleet->getSize();
    }
    
    if(playerid1 == 0){
      objectmanager->doneWithObject(ob->getID());
      continue;
    }
    
    
    for(std::set<unsigned int>::iterator itb = itcurr; itb != possiblecombatants.end(); ++itb){
      IGObject* itbobj = objectmanager->getObject(*itb);
      uint32_t playerid2;
      Vector3d pos2;
      uint32_t size2;
      if(itbobj->getType() == planettype){
        Planet* planet = (Planet*)(itbobj->getObjectData());
        playerid2 = planet->getOwner();
        pos2 = planet->getPosition();
        size2 = planet->getSize();
      }else{
        Fleet* fleet = (Fleet*)(itbobj->getObjectData());
        playerid2 = fleet->getOwner();
        pos2 = fleet->getPosition();
        size2 = fleet->getSize();
      }
      
      if(playerid2 == 0 || playerid1 == playerid2){
        objectmanager->doneWithObject(itbobj->getID());
        continue;
      }

      uint64_t diff = pos1.getDistance(pos2);
      if(diff <= size1 / 2 + size2 / 2){
        combatstrategy->setCombatants(ob, itbobj);
        combatstrategy->doCombat();
        if(!combatstrategy->isAliveCombatant1()){
          if(ob->getType() == planettype){
            uint32_t oldowner = ((Planet*)(ob->getObjectData()))->getOwner();
            ((Planet*)(ob->getObjectData()))->setOwner(0);
            uint32_t queueid = static_cast<OrderQueueObjectParam*>(ob->getObjectData()->getParameterByType(obpT_Order_Queue))->getQueueId();
            OrderQueue* queue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
            queue->removeOwner(oldowner);
            queue->removeAllOrders();
          }else{
            objectmanager->scheduleRemoveObject(*itcurr);
          }
        }
        if(!combatstrategy->isAliveCombatant2()){
          if(itbobj->getType() == planettype){
            uint32_t oldowner = ((Planet*)(itbobj->getObjectData()))->getOwner();
            ((Planet*)(itbobj->getObjectData()))->setOwner(0);
            uint32_t queueid = static_cast<OrderQueueObjectParam*>(itbobj->getObjectData()->getParameterByType(obpT_Order_Queue))->getQueueId();
            OrderQueue* queue = Game::getGame()->getOrderManager()->getOrderQueue(queueid);
            queue->removeOwner(oldowner);
            queue->removeAllOrders();
          }else{
            objectmanager->scheduleRemoveObject(*itb);
          }
        }
      }
      objectmanager->doneWithObject(itbobj->getID());
    }
    objectmanager->doneWithObject(ob->getID());
  }

  objectmanager->clearRemovedObjects();
  
  // do other orders (nop, buildfleet, colonise)
  
  for(itcurr = otherorders.begin(); itcurr != otherorders.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    if(ob != NULL && (ob->getType() == planettype || ob->getType() == fleettype)){
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getObjectData()->getParameterByType(obpT_Order_Queue));
      if(oqop != NULL){
        OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
        if(orderqueue != NULL){
          Order * currOrder = orderqueue->getFirstOrder();
          if(currOrder != NULL){
            if(currOrder->doOrder(ob)){
              orderqueue->removeFirstOrder();
            }else{
              orderqueue->updateFirstOrder();
            }
          }
        }
      }
    }
    objectmanager->doneWithObject(ob->getID());
  }

  objectmanager->clearRemovedObjects();
  
  // to once a turn (right at the end)
  objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject * ob = objectmanager->getObject(*itcurr);
    ob->getObjectData()->doOnceATurn(ob);
    objectmanager->doneWithObject(ob->getID());
  }

  // find the objects that are visible to each player
  std::set<uint32_t> vis = objectmanager->getAllIds();
  std::set<uint32_t> players = playermanager->getAllIds();
  for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
      playermanager->getPlayer(*itplayer)->getPlayerView()->setVisibleObjects(vis);
  }
  playermanager->updateAll();
  
  delete combatstrategy;
}

void MinisecTurn::setPlanetType(uint32_t pt){
  planettype = pt;
}

void MinisecTurn::setFleetType(uint32_t ft){
  fleettype = ft;
}

std::set<uint32_t> MinisecTurn::getContainerIds() const{
  return containerids;
}
