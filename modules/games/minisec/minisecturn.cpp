/*  MinisecTurn object
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

#include <algorithm>

#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/playermanager.h>
#include <tpserver/order.h>
#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include "planet.h"
#include "fleet.h"
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/ordermanager.h>
#include <tpserver/message.h>

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
  PlayerManager::Ptr playermanager = game->getPlayerManager();
 
  //sort by order type
  std::set<uint32_t> movers;
  std::set<uint32_t> otherorders;
  std::set<uint32_t> interceptors; 
  
  containerids.clear();
  std::set<uint32_t> possiblecombatants;
  
  std::set<uint32_t> objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject::Ptr ob = objectmanager->getObject(*itcurr);
    if(ob->getType() == planettype || ob->getType() == fleettype){
      possiblecombatants.insert(ob->getID());
      OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
      if(oqop != NULL){
        OrderQueue::Ptr orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
        if(orderqueue != NULL){
          Order * currOrder = orderqueue->getFirstOrder();
          if(currOrder != NULL){
            if(currOrder->getType() == ordermanager->getOrderTypeByName("Move")){
              movers.insert(ob->getID());
            } else if (currOrder->getType() == ordermanager->getOrderTypeByName("Intercept")){
              interceptors.insert(ob->getID());
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
    IGObject::Ptr ob = objectmanager->getObject(*itcurr);
    
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    OrderQueue::Ptr orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
    Order * currOrder = orderqueue->getFirstOrder();
    if(currOrder->doOrder(ob)){
      orderqueue->removeFirstOrder();
    }else{
      orderqueue->updateFirstOrder();
    }
    
    objectmanager->doneWithObject(ob->getID());
  }

  // do interceptions

  for(itcurr = interceptors.begin(); itcurr != interceptors.end(); ++itcurr) {
    IGObject::Ptr ob = objectmanager->getObject(*itcurr);
    
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
    OrderQueue::Ptr orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
    Order * currOrder = orderqueue->getFirstOrder();
    if(currOrder->doOrder(ob)){
      orderqueue->removeFirstOrder();
    }else{
      orderqueue->updateFirstOrder();
    }
    
    objectmanager->doneWithObject(ob->getID());
  }
 
  // do combat
  
  std::list<std::map<uint32_t, std::set<uint32_t> > > combats;
  
  for(itcurr = possiblecombatants.begin(); itcurr != possiblecombatants.end(); ++itcurr) {
    IGObject::Ptr ob = objectmanager->getObject(*itcurr);
    uint32_t playerid1;
    Vector3d pos1;
    uint32_t size1;
    if(ob->getType() == planettype){
      Planet* planet = (Planet*)(ob->getObjectBehaviour());
      playerid1 = planet->getOwner();
      pos1 = planet->getPosition();
      size1 = planet->getSize();
    }else{
      Fleet* fleet = (Fleet*)(ob->getObjectBehaviour());
      playerid1 = fleet->getOwner();
      pos1 = fleet->getPosition();
      size1 = fleet->getSize();
    }
    
    if(playerid1 == 0){
      objectmanager->doneWithObject(ob->getID());
      continue;
    }
    
    bool placed = false;
    
    for(std::list<std::map<uint32_t, std::set<uint32_t> > >::iterator itlist = combats.begin();
        itlist != combats.end(); ++itlist){
      std::map<uint32_t, std::set<uint32_t> > themap = *itlist;
      for(std::map<uint32_t, std::set<uint32_t> >::iterator itmap = themap.begin();
          itmap != themap.end(); ++itmap){
        std::set<uint32_t> theset = itmap->second;
        for(std::set<uint32_t>::iterator itset = theset.begin(); itset != theset.end(); ++itset){
          IGObject::Ptr itbobj = objectmanager->getObject(*itset);
          uint32_t playerid2;
          Vector3d pos2;
          uint32_t size2;
          if(itbobj->getType() == planettype){
            Planet* planet = (Planet*)(itbobj->getObjectBehaviour());
            playerid2 = planet->getOwner();
            pos2 = planet->getPosition();
            size2 = planet->getSize();
          }else{
            Fleet* fleet = (Fleet*)(itbobj->getObjectBehaviour());
            playerid2 = fleet->getOwner();
            pos2 = fleet->getPosition();
            size2 = fleet->getSize();
          }
        
          if(playerid2 == 0){
            objectmanager->doneWithObject(itbobj->getID());
            continue;
          }
          uint64_t diff = pos1.getDistance(pos2);
          if(diff <= size1 / 2 + size2 / 2){
            themap[playerid1].insert(ob->getID());
            *itlist = themap;
            placed = true;
            objectmanager->doneWithObject(itbobj->getID());
            break;
          }
          objectmanager->doneWithObject(itbobj->getID());
        }
        if(placed)
          break;
      }
      if(placed)
        break;
    }
    if(!placed){
      std::map<uint32_t, std::set<uint32_t> > themap;
      std::set<uint32_t> theset;
      theset.insert(ob->getID());
      themap[playerid1] = theset;
      combats.push_back(themap);
    }
    objectmanager->doneWithObject(ob->getID());
  }
  
  for(std::list<std::map<uint32_t, std::set<uint32_t> > >::iterator itlist = combats.begin();
        itlist != combats.end(); ++itlist){
    std::map<uint32_t, std::set<uint32_t> > themap = *itlist;
    if(themap.size() >= 2){
      combatstrategy->doCombat(themap);
    }
  }
  

  objectmanager->clearRemovedObjects();
  
  // do other orders (nop, buildfleet, colonise)
  
  for(itcurr = otherorders.begin(); itcurr != otherorders.end(); ++itcurr) {
    IGObject::Ptr ob = objectmanager->getObject(*itcurr);
    if(ob != NULL){
      if(ob->getType() == planettype || ob->getType() == fleettype){
        OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(ob->getParameterByType(obpT_Order_Queue));
        if(oqop != NULL){
          OrderQueue::Ptr orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
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
  }

  objectmanager->clearRemovedObjects();
  
  // to once a turn (right at the end)
  objects = objectmanager->getAllIds();
  for(itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject::Ptr ob = objectmanager->getObject(*itcurr);
    if(ob->isAlive()){
      ob->getObjectBehaviour()->doOnceATurn();
    }
    objectmanager->doneWithObject(ob->getID());
  }

  // find the objects that are visible to each player
  std::set<uint32_t> vis = objectmanager->getAllIds();
  std::set<uint32_t> players = playermanager->getAllIds();
  uint32_t numaliveplayers = 0;
  uint32_t numdeadplayers = 0;
  for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
    Player::Ptr player = playermanager->getPlayer(*itplayer);
    PlayerView::Ptr playerview = player->getPlayerView();
    
    for(std::set<uint32_t>::iterator itob = vis.begin(); itob != vis.end(); ++itob){
      ObjectView::Ptr obv = playerview->getObjectView(*itob);
      if(!obv){
        if(objectmanager->getObject(*itob)->isAlive()){
          playerview->addVisibleObject( *itob, true );
        }
        objectmanager->doneWithObject(*itob);
      }else{
        IGObject::Ptr ro = objectmanager->getObject(*itob);
        uint64_t obmt = ro->getModTime();
        objectmanager->doneWithObject(*itob);
        if(obmt > obv->getModTime()){
          obv->setModTime(obmt);
          playerview->updateObjectView(*itob);
        }
      }
    }
    
    // remove dead objects
    std::set<uint32_t> goneobjects;
    std::set<uint32_t> knownobjects = playerview->getVisibleObjects();
    set_difference(knownobjects.begin(), knownobjects.end(), vis.begin(), vis.end(), inserter(goneobjects, goneobjects.begin()));
    
    for(std::set<uint32_t>::iterator itob = goneobjects.begin(); itob != goneobjects.end(); ++itob){
      ObjectView::Ptr obv = playerview->getObjectView(*itob);
        if(!obv->isGone()){
            obv->setGone(true);
            playerview->updateObjectView(*itob);
        }
    }

    
    if(!player->isAlive() || playerview->getNumberOwnedObjects() == 0){
      if(player->isAlive()){
        Message::Ptr msg( new Message() );
        msg->setSubject("You lost");
        msg->setBody("You do not own any objects, therefore you game has finished.");
        msg->addReference(rst_Action_Player, rspav_Eliminated);
        player->postToBoard(msg);
        player->setIsAlive(false);
      }
      numdeadplayers++;
    }else{
      numaliveplayers++;
    }
  }
  
  if(numaliveplayers == 1){
    //find alive player
    Player::Ptr player;
    for(std::set<uint32_t>::iterator itplayer = players.begin(); itplayer != players.end(); ++itplayer){
      player = playermanager->getPlayer(*itplayer);
      if(player->isAlive())
        break;
    }
    if(player->getScore(0) != numdeadplayers - 1){
      Message::Ptr msg( new Message() );
      msg->setSubject("You won!");
      msg->setBody("You have eliminated all the competing players. Congratulations!");
      player->postToBoard(msg);
      player->setScore(0, numdeadplayers - 1);
    }
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
