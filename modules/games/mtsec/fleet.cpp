/*  Fleet object
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <math.h>

#include <tpserver/order.h>
#include <tpserver/game.h>
#include "planet.h"
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/refquantitylistobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/logging.h>
#include <tpserver/resourcelistobjectparam.h>

#include "fleet.h"

namespace MTSecRuleset {

FleetType::FleetType():OwnedObjectType( "Fleet", "Fleet of ships"){
  ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc( "Ships", "The information about ships in this fleet");
  group->addParameter(obpT_Reference_Quantity_List, "Ship List", "The list of ships");
  group->addParameter(obpT_Integer, "Damage", "The damage done to the ships");

  group = createParameterGroupDesc("Resources", "The fleet's weapon resources");
  group->addParameter(obpT_Resource_List, "Weapon Resource List", "The weapon list the fleet has available");

}

FleetType::~FleetType(){
}

ObjectBehaviour* FleetType::createObjectBehaviour() const{
  return new Fleet();
}

Fleet::Fleet():OwnedObject(){
}

Fleet::~Fleet(){
}

void Fleet::setDefaultOrderTypes(){
  OrderManager * om = Game::getGame()->getOrderManager();
  std::set<uint32_t> allowedlist;
  allowedlist.insert(om->getOrderTypeByName("No Operation"));
  allowedlist.insert(om->getOrderTypeByName("Move"));
  allowedlist.insert(om->getOrderTypeByName("Split Fleet"));
  allowedlist.insert(om->getOrderTypeByName("Merge Fleet"));
  allowedlist.insert(om->getOrderTypeByName("Load Armament"));
  allowedlist.insert(om->getOrderTypeByName("Unload Armament"));
  dynamic_cast<OrderQueueObjectParam*>(obj->getParameter(3,1))->setAllowedOrders(allowedlist);
}

void Fleet::addShips(uint32_t type, uint32_t number){
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  ships[std::pair<int32_t, uint32_t>(rst_Design, type)] += number;
  ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);
  DesignStore::Ptr ds = Game::getGame()->getDesignStore();
  OrderManager* om = Game::getGame()->getOrderManager();
  if(ds->getDesign(type)->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.0){
    std::set<uint32_t> allowed = dynamic_cast<OrderQueueObjectParam*>(obj->getParameter(3,1))->getAllowedOrders();
    allowed.insert(om->getOrderTypeByName("Colonise"));
    dynamic_cast<OrderQueueObjectParam*>(obj->getParameter(3,1))->setAllowedOrders(allowed);
  }
  obj->touchModTime();
}

bool Fleet::removeShips(uint32_t type, uint32_t number){
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  if(ships[std::pair<int32_t, uint32_t>(rst_Design, type)] >= number){
    ships[std::pair<int32_t, uint32_t>(rst_Design, type)] -= number;
    if(ships[std::pair<int32_t, uint32_t>(rst_Design, type)] == 0){
      ships.erase(std::pair<int32_t, uint32_t>(rst_Design, type));
    }
    ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);
    DesignStore::Ptr ds = Game::getGame()->getDesignStore();
    bool colonise = false;
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin();
        itcurr != ships.end(); ++itcurr){
      if(ds->getDesign(itcurr->first.second)->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.0){
        colonise = true;
        break;
      }
    }
    OrderManager* om = Game::getGame()->getOrderManager();
    OrderQueueObjectParam* orderqueue = dynamic_cast<OrderQueueObjectParam*>(obj->getParameter(3,1));
    if(colonise){
      std::set<uint32_t> allowed = orderqueue->getAllowedOrders();
      allowed.insert(om->getOrderTypeByName("Colonise"));
      orderqueue->setAllowedOrders(allowed);
    }else{
      std::set<uint32_t> allowed = orderqueue->getAllowedOrders();
      allowed.erase(om->getOrderTypeByName("Colonise"));
      orderqueue->setAllowedOrders(allowed);
    }
    obj->touchModTime();
    return true;
  }
  return false;
}

uint32_t Fleet::numShips(uint32_t type){
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  return ships[std::pair<int32_t, uint32_t>(rst_Design, type)];
}

std::map<uint32_t, uint32_t> Fleet::getShips() const{
  std::map<uint32_t, uint32_t> ships;
  std::map<std::pair<int32_t, uint32_t>, uint32_t> shipsref = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = shipsref.begin();
      itcurr != shipsref.end(); ++itcurr){
    ships[itcurr->first.second] = itcurr->second;
  }
  return ships;
}

int64_t Fleet::maxSpeed(){
  Logger::getLogger()->debug("Enter Fleet::maxSpeed");
  double speed = 1e100;
  DesignStore::Ptr ds = Game::getGame()->getDesignStore();
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
        speed = fmin(speed, ds->getDesign(itcurr->first.second)->getPropertyValue(ds->getPropertyByName("Speed")));
  }
  Logger::getLogger()->debug("Exit Fleet::maxSpeed");
  return (int64_t)(floor(speed));
}

uint32_t Fleet::totalShips() const{
  uint32_t num = 0;
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}

int32_t Fleet::getDesignId(uint32_t id) const {
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
        itcurr != ships.end(); ++itcurr)
    {
        if (itcurr->first.second == id)
            return itcurr->first.first;
    }
    return -1;
}


uint32_t Fleet::getDamage() const{
    return dynamic_cast<IntegerObjectParam*>(obj->getParameter(4,2))->getValue();
}

void Fleet::setDamage(uint32_t nd){
    dynamic_cast<IntegerObjectParam*>(obj->getParameter(4,2))->setValue(nd);
    obj->touchModTime();
}

void Fleet::packExtraData(OutputFrame::Ptr frame){
  OwnedObject::packExtraData(frame);

  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = dynamic_cast<RefQuantityListObjectParam*>(obj->getParameter(4,1))->getRefQuantityList();
  frame->packInt(ships.size());
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
    //if(itcurr->second > 0){
      frame->packInt(itcurr->first.second);
      frame->packInt(itcurr->second);
      //}
  }
  
  frame->packInt(dynamic_cast<IntegerObjectParam*>(obj->getParameter(4,2))->getValue());

}

void Fleet::doOnceATurn(){
  Game* game = Game::getGame();
  IGObject::Ptr pob = game->getObjectManager()->getObject(obj->getParent());
  uint32_t obT_Planet = game->getObjectTypeManager()->getObjectTypeByName("Planet");
  if(pob->getType() == obT_Planet && dynamic_cast<Planet*>(pob->getObjectBehaviour())->getOwner() == getOwner()){
    IntegerObjectParam* damage = dynamic_cast<IntegerObjectParam*>(obj->getParameter(4,2));
    if(damage->getValue() != 0){
        damage->setValue(0);
        obj->touchModTime();
    }
  }
  game->getObjectManager()->doneWithObject(obj->getParent());
  
  Order* order = game->getOrderManager()->getOrderQueue(dynamic_cast<OrderQueueObjectParam*>(obj->getParameter(3,1))->getQueueId())->getFirstOrder();
  if(order == NULL || order->getType() != game->getOrderManager()->getOrderTypeByName("Move")){
    setVelocity(Vector3d(0,0,0));
  }
}

std::map<uint32_t, std::pair<uint32_t, uint32_t> > Fleet::getResources() {
    return dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->getResources();
}

void Fleet::addResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->getResources();
    if(reslist.find(restype) != reslist.end()){
      std::pair<uint32_t, uint32_t> respair = reslist[restype];
      respair.first += amount;
      reslist[restype] = respair;
      dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->setResources(reslist);
    } else {
      setResource(restype,amount);
    }
}

void Fleet::setResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->getResources();
    std::pair<uint32_t, uint32_t> respair = reslist[restype];
    respair.first = amount;
    reslist[restype] = respair;
    dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->setResources(reslist);
    obj->touchModTime();
}

bool Fleet::removeResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->getResources();
    if(reslist.find(restype) != reslist.end()){
        if(reslist[restype].first >= amount){
            std::pair<uint32_t, uint32_t> respair = reslist[restype];
            respair.first -= amount;
            reslist[restype] = respair;
            dynamic_cast<ResourceListObjectParam*>(obj->getParameter(5,1))->setResources(reslist);
            obj->touchModTime();
            return true;
        }
    }
    return false;
}

int Fleet::getContainerType(){
  return 0;
}

void Fleet::setupObject(){
  OwnedObject::setupObject();
  setSize(2);
  //something about the orderqueue?
}

}
