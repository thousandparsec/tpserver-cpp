/*  Planet objects
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/frame.h>
#include <tpserver/order.h>
#include <tpserver/ordermanager.h>
#include <tpserver/game.h>
#include <tpserver/resourcelistobjectparam.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/objectparametergroup.h>

#include "planet.h"

Planet::Planet():OwnedObject(){
  Position3dObjectParam * pos = new Position3dObjectParam();
  pos->setName("Position");
  pos->setDescription("The position of the planet");
  ObjectParameterGroup* group = new ObjectParameterGroup();
  group->setGroupId(2);
  group->setName("Positional");
  group->setDescription("Positional information");
  group->addParameter(pos);
  SizeObjectParam* size = new SizeObjectParam();
  size->setName("Size");
  size->setDescription( "The size of the planet");
  size->setSize(2);
  group->addParameter(size);
  paramgroups.push_back(group);
  
  group = new ObjectParameterGroup();
  group->setGroupId(3);
  group->setName("Orders");
  group->setDescription("The order queues of the planet");
  orderqueue = new OrderQueueObjectParam();
  orderqueue->setName("Order Queue");
  orderqueue->setDescription("The queue of orders for this planet");
  
  group->addParameter(orderqueue);
  paramgroups.push_back(group);
  
  group = new ObjectParameterGroup();
  group->setGroupId(4);
  group->setName("Resources");
  group->setDescription("The planets resources");
  resources = new ResourceListObjectParam();
  resources->setName("Resource List");
  resources->setDescription("The resource list of the resources the planet has available");
  group->addParameter(resources);
  paramgroups.push_back(group);
  
  nametype = "Planet";
  typedesc = "A planet object";
}

void Planet::setDefaultOrderTypes(){
  OrderManager * om = Game::getGame()->getOrderManager();
  std::set<uint32_t> allowedlist;
  allowedlist.insert(om->getOrderTypeByName("BuildFleet"));
  allowedlist.insert(om->getOrderTypeByName("No Operation"));
  orderqueue->setAllowedOrders(allowedlist);
}

void Planet::packExtraData(Frame * frame){
  OwnedObject::packExtraData(frame);
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = resources->getResources();
    frame->packInt(reslist.size());
    for(std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator itcurr = reslist.begin();
            itcurr != reslist.end(); ++itcurr){
        frame->packInt(itcurr->first);
        frame->packInt(itcurr->second.first);
        frame->packInt(itcurr->second.second);
        frame->packInt(0);
    }
}

void Planet::doOnceATurn(IGObject * obj)
{

}

void Planet::packAllowedOrders(Frame * frame, int playerid){
  if(playerid == getOwner()){
    frame->packInt(2);
    OrderManager * om = Game::getGame()->getOrderManager();
    frame->packInt(om->getOrderTypeByName("BuildFleet"));
    frame->packInt(om->getOrderTypeByName("No Operation"));
  }else{
    frame->packInt(0);
  }
}

bool Planet::checkAllowedOrder(int ot, int playerid){
  OrderManager * om = Game::getGame()->getOrderManager();
  return (playerid == getOwner() && (ot == om->getOrderTypeByName("BuildFleet") || ot == om->getOrderTypeByName("No Operation")));
}

int Planet::getContainerType(){
  return 2;
}

ObjectData* Planet::clone(){
  return new Planet();
}

std::map<uint32_t, std::pair<uint32_t, uint32_t> > Planet::getResources(){
    return resources->getResources();
}

uint32_t Planet::getResource(uint32_t restype) const{
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = resources->getResources();
  if(reslist.find(restype) != reslist.end()){
    return reslist.find(restype)->first;
  }
  return 0;
}

void Planet::setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress){
    resources->setResources(ress);
}

void Planet::addResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = resources->getResources();
    std::pair<uint32_t, uint32_t> respair = reslist[restype];
    respair.first += amount;
    reslist[restype] = respair;
    resources->setResources(reslist);
}

bool Planet::removeResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = resources->getResources();
    if(reslist.find(restype) != reslist.end()){
        if(reslist[restype].first >= amount){
            std::pair<uint32_t, uint32_t> respair = reslist[restype];
            respair.first -= amount;
            reslist[restype] = respair;
            resources->setResources(reslist);
            return true;
        }
    }
    return false;
}
