/*  Fleet object
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/frame.h>
#include <tpserver/order.h>
#include <tpserver/game.h>
#include "planet.h"
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/logging.h>
#include <tpserver/ordermanager.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/velocity3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/refquantitylistobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroup.h>
#include <tpserver/refsys.h>

#include "fleet.h"

Fleet::Fleet():OwnedObject(){
  pos = new Position3dObjectParam();
  pos->setName("Position");
  pos->setDescription("The position of the Fleet");
  ObjectParameterGroup* group = new ObjectParameterGroup();
  group->setGroupId(2);
  group->setName("Positional");
  group->setDescription("Positional information");
  group->addParameter(pos);
  vel = new Velocity3dObjectParam();
  vel->setName("Velocity");
  vel->setDescription("The velocity of the fleet");
  group->addParameter(vel);
  SizeObjectParam* size = new SizeObjectParam();
  size->setName("Size");
  size->setDescription( "The size of the planet");
  size->setSize(2);
  group->addParameter(size);
  paramgroups.push_back(group);
  
  group = new ObjectParameterGroup();
  group->setGroupId(3);
  group->setName("Orders");
  group->setDescription("The order queues of the fleet");
  orderqueue = new OrderQueueObjectParam();
  orderqueue->setName("Order Queue");
  orderqueue->setDescription("The queue of orders for this fleet");
  OrderManager * om = Game::getGame()->getOrderManager();
  group->addParameter(orderqueue);
  paramgroups.push_back(group);
  
  group = new ObjectParameterGroup();
  group->setGroupId(4);
  group->setName("Ships");
  group->setDescription("The information about ships in this fleet");
  shiplist = new RefQuantityListObjectParam();
  shiplist->setName("Ship List");
  shiplist->setDescription("The list of ships");
  group->addParameter(shiplist);
  damage = new IntegerObjectParam();
  damage->setName("Damage");
  damage->setDescription("The damage done to the ships");
  damage->setValue(0);
  group->addParameter(damage);
  paramgroups.push_back(group);
  
  nametype = "Fleet";
  typedesc = "Fleet of ships";
}

Fleet::~Fleet(){
}

Vector3d Fleet::getPosition() const{
  return pos->getPosition();
}

Vector3d Fleet::getVelocity() const{
  return vel->getVelocity();
}

void Fleet::setPosition(const Vector3d & np){
  pos->setPosition(np);
  touchModTime();
}

void Fleet::setVelocity(const Vector3d & nv){
  vel->setVelocity(nv);
  touchModTime();
}

void Fleet::setDefaultOrderTypes(){
  OrderManager * om = Game::getGame()->getOrderManager();
  std::set<uint32_t> allowedlist;
  allowedlist.insert(om->getOrderTypeByName("No Operation"));
  allowedlist.insert(om->getOrderTypeByName("Move"));
  allowedlist.insert(om->getOrderTypeByName("SplitFleet"));
  allowedlist.insert(om->getOrderTypeByName("MergeFleet"));
  orderqueue->setAllowedOrders(allowedlist);
}

void Fleet::addShips(int type, int number){
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = shiplist->getRefQuantityList();
  ships[std::pair<int32_t, uint32_t>(rst_Design, type)] += number;
  shiplist->setRefQuantityList(ships);
  DesignStore* ds = Game::getGame()->getDesignStore();
  OrderManager* om = Game::getGame()->getOrderManager();
  if(ds->getDesign(type)->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.0){
    std::set<uint32_t> allowed = orderqueue->getAllowedOrders();
    allowed.insert(om->getOrderTypeByName("Colonise"));
    orderqueue->setAllowedOrders(allowed);
  }
  touchModTime();
}

bool Fleet::removeShips(int type, int number){
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = shiplist->getRefQuantityList();
  if(ships[std::pair<int32_t, uint32_t>(rst_Design, type)] >= number){
    ships[std::pair<int32_t, uint32_t>(rst_Design, type)] -= number;
    if(ships[std::pair<int32_t, uint32_t>(rst_Design, type)] == 0){
      ships.erase(std::pair<int32_t, uint32_t>(rst_Design, type));
    }
    shiplist->setRefQuantityList(ships);
    DesignStore* ds = Game::getGame()->getDesignStore();
    bool colonise = false;
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin();
        itcurr != ships.end(); ++itcurr){
      if(ds->getDesign(itcurr->first.second)->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.0){
        colonise = true;
        break;
      }
    }
    OrderManager* om = Game::getGame()->getOrderManager();
    if(colonise){
      std::set<uint32_t> allowed = orderqueue->getAllowedOrders();
      allowed.insert(om->getOrderTypeByName("Colonise"));
      orderqueue->setAllowedOrders(allowed);
    }else{
      std::set<uint32_t> allowed = orderqueue->getAllowedOrders();
      allowed.erase(om->getOrderTypeByName("Colonise"));
      orderqueue->setAllowedOrders(allowed);
    }
    touchModTime();
    return true;
  }
  return false;
}

int Fleet::numShips(int type){
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = shiplist->getRefQuantityList();
  return ships[std::pair<int32_t, uint32_t>(rst_Design, type)];
}

std::map<int, int> Fleet::getShips() const{
  std::map<int, int> ships;
  std::map<std::pair<int32_t, uint32_t>, uint32_t> shipsref = shiplist->getRefQuantityList();
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = shipsref.begin();
      itcurr != shipsref.end(); ++itcurr){
    ships[itcurr->first.second] = itcurr->second;
  }
  return ships;
}

int Fleet::totalShips() const{
  int num = 0;
  std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = shiplist->getRefQuantityList();
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}


int Fleet::getDamage() const{
    return damage->getValue();
}

void Fleet::setDamage(int nd){
    damage->setValue(nd);
    touchModTime();
}

void Fleet::packExtraData(Frame * frame)
{
	OwnedObject::packExtraData(frame);
	
        std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = shiplist->getRefQuantityList();
	frame->packInt(ships.size());
        for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
	  //if(itcurr->second > 0){
	    frame->packInt(itcurr->first.second);
	    frame->packInt(itcurr->second);
	    //}
	}
	
	frame->packInt(damage->getValue());

}

void Fleet::doOnceATurn(IGObject * obj)
{
  IGObject * pob = Game::getGame()->getObjectManager()->getObject(obj->getParent());
  if(pob->getType() == obT_Planet && ((Planet*)(pob->getObjectData()))->getOwner() == getOwner()){
    if(damage->getValue() != 0){
        damage->setValue(0);
        touchModTime();
    }
  }
    Game::getGame()->getObjectManager()->doneWithObject(obj->getParent());
}

int Fleet::getContainerType(){
  return 0;
}

ObjectData* Fleet::clone(){
  return new Fleet();
}
