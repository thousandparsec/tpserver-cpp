/*  Planet objects
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

#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/order.h>
#include <tpserver/ordermanager.h>
#include <tpserver/game.h>
#include <tpserver/resourcelistobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/logging.h>
#include <tpserver/integerobjectparam.h>

#include "planet.h"

namespace MTSecRuleset {

PlanetType::PlanetType():OwnedObjectType("Planet", "A planet object"){
  ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
  group->setName("Resources");
  group->setDescription("The planet's resources");
  group->addParameter(obpT_Resource_List, "Resource List", "The resource list of the resources the planet has available");
  addParameterGroupDesc(group);

  group = new ObjectParameterGroupDesc();
  group->setName("Factories");
  group->setDescription("The planet's factories");
  group->addParameter(obpT_Integer, "Factories", "The factories that the planet has available");
  group->addParameter(obpT_Integer, "One Time", "The factories to be enhanced one time only");
  addParameterGroupDesc(group);

  nametype = "Planet";
  typedesc = "A planet object";
}

PlanetType::~PlanetType(){
}

ObjectBehaviour* PlanetType::createObjectBehaviour() const{
  return new Planet();
}


Planet::Planet()
{
}

Planet::~Planet(){
}

void Planet::setDefaultOrderTypes(){
  OrderManager * om = Game::getGame()->getOrderManager();
  std::set<uint32_t> allowedlist;
  allowedlist.insert(om->getOrderTypeByName("Build Fleet"));
  allowedlist.insert(om->getOrderTypeByName("Enhance"));
  allowedlist.insert(om->getOrderTypeByName("No Operation"));
  allowedlist.insert(om->getOrderTypeByName("Send Points"));
  allowedlist.insert(om->getOrderTypeByName("Build Weapon"));
  dynamic_cast<OrderQueueObjectParam*>(obj->getParameter(3,1))->setAllowedOrders(allowedlist);
  Game* game = Game::getGame();
  ResourceManager* resman = game->getResourceManager();
  const uint32_t restype = resman->getResourceDescription("Factories")->getResourceType();
  dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->setValue(getResourceSurfaceValue(restype));
  dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,2))->setValue(0);
}

void Planet::packExtraData(Frame * frame){
  OwnedObject::packExtraData(frame);
  
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
    frame->packInt(reslist.size());
    for(std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator itcurr = reslist.begin();
            itcurr != reslist.end(); ++itcurr){
        frame->packInt(itcurr->first);
        frame->packInt(itcurr->second.first);
        frame->packInt(itcurr->second.second);
        frame->packInt(0);
    }
  //unneeded as per llnz, doesn't allow client to connect
  //frame->packInt(dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->getValue());

}

void Planet::doOnceATurn()
{
  if (getOwner() != 0) {
    uint32_t factories = dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->getValue();
    ResourceManager* resman = Game::getGame()->getResourceManager();
    const uint32_t restype = resman->getResourceDescription("Factories")->getResourceType();

    if (factories < 100)
    {
      Logger::getLogger()->debug("Planet::doOnceATurn Factories(%d) less than 100, incrementing by 1", factories);
      factories++;
      dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->setValue(factories);
      //reset factories resource every turn, assume all points used
      setResource(restype, factories);
    }
    // Add the once-per-turn points
    const uint32_t sentPoints = dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,2))->getValue();
    Logger::getLogger()->debug("Planet::doOnceATurn Adding once-per-turn points: %d", sentPoints);
    addResource(restype, sentPoints);
    dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,2))->setValue(0);
  }
}

int Planet::getContainerType(){
  return 2;
}



std::map<uint32_t, std::pair<uint32_t, uint32_t> > Planet::getResources(){
    return dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
}

uint32_t Planet::getResource(uint32_t restype) const{
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
  if(reslist.find(restype) != reslist.end()){
    return reslist.find(restype)->first;
  }
  return 0;
}

uint32_t Planet::getResourceSurfaceValue(uint32_t restype) const {
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
  if(reslist.find(restype) != reslist.end()){
    return reslist.find(restype)->second.first;
  }
  return 0;
}

void Planet::setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress){
    dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->setResources(ress);
    obj->touchModTime();
}

uint32_t Planet::getFactoriesPerTurn(){
    return dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->getValue();
}

void Planet::addResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
    std::pair<uint32_t, uint32_t> respair = reslist[restype];
    respair.first += amount;
    reslist[restype] = respair;
    dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->setResources(reslist);
    obj->touchModTime();
}

void Planet::setResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
    std::pair<uint32_t, uint32_t> respair = reslist[restype];
    respair.first = amount;
    reslist[restype] = respair;
    dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->setResources(reslist);
    obj->touchModTime();
}

void Planet::addFactoriesNextTurn(uint32_t amount){
  dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,2))->setValue(amount);
}

void Planet::addFactories(uint32_t amount){
  int32_t current = dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->getValue();
  dynamic_cast<IntegerObjectParam*>(obj->getParameter(5,1))->setValue(current+amount);
}

bool Planet::removeResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->getResources();
    if(reslist.find(restype) != reslist.end()){
        if(reslist[restype].first >= amount){
            std::pair<uint32_t, uint32_t> respair = reslist[restype];
            respair.first -= amount;
            reslist[restype] = respair;
            dynamic_cast<ResourceListObjectParam*>(obj->getParameter(4,1))->setResources(reslist);
            obj->touchModTime();
            return true;
        }
    }
    return false;
}

}
