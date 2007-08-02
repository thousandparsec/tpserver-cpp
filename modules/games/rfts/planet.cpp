/*  Planet
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

#include <map>

#include <tpserver/frame.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/resourcelistobjectparam.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroup.h>
#include <tpserver/game.h>
#include <tpserver/ordermanager.h>

#include "containertypes.h"
#include "nop.h"

#include "planet.h"


namespace RFTS_ {

using std::string;

Planet::Planet() : StaticObject() {
   nametype = "Planet";
   typedesc = "A planet object";
   setSize(2);

   ObjectParameterGroup* group = new ObjectParameterGroup();
   
   playerref = new ReferenceObjectParam();
   playerref->setName("Owner");
   playerref->setDescription("The owner of this object");
   playerref->setReferenceType(rst_Player);
   group = new ObjectParameterGroup();
   group->setGroupId(2);
   group->setName("Ownership");
   group->setDescription("The ownership of this object");
   group->addParameter(playerref);
   paramgroups.push_back(group);
      
   group = new ObjectParameterGroup();
   group->setGroupId(3);
   group->setName("Resources");
   group->setDescription("The planets resources");
   resources = new ResourceListObjectParam();
   resources->setName("Resource List");
   resources->setDescription("The resource list of the resources the planet has available");
   group->addParameter(resources);
   paramgroups.push_back(group);
   
   group = new ObjectParameterGroup();
   group->setGroupId(4);
   group->setName("Orders");
   group->setDescription("The order queues of the planet");
   orderqueue = new OrderQueueObjectParam();
   orderqueue->setName("Order Queue");
   orderqueue->setDescription("The queue of orders for this planet");
   
   group->addParameter(orderqueue);
   paramgroups.push_back(group); 

   // could take the place of resources
   group = new ObjectParameterGroup();
   group->setGroupId(5);
   group->setName("Stats");
   group->setDescription("The planet's stats");
   resourcePoints = new IntegerObjectParam();
   resourcePoints->setName("Resource Points");
   resourcePoints->setDescription("The resource points for this planet");
   resourcePoints->setValue(0);
   group->addParameter(resourcePoints);
   
   paramgroups.push_back(group);
   
}

Planet::~Planet() {

}

void Planet::setDefaultOrderTypes() {
   OrderManager *om = Game::getGame()->getOrderManager();
   std::set<uint32_t> allowedlist;
   allowedlist.insert(om->getOrderTypeByName("No Operation"));
   orderqueue->setAllowedOrders(allowedlist);
}

void Planet::doOnceATurn(IGObject* obj) {
   if(!(Game::getGame()->getTurnNumber() % 2))
   {
      // todo, calc resource points etc
   }
}

int Planet::getContainerType() {
   return ContainerTypes_::Planet;
}

ObjectData* Planet::clone() const {
   return new Planet();
}

void Planet::packExtraData(Frame * frame){
  ObjectData::packExtraData(frame);
  
  frame->packInt((playerref->getReferencedId() == 0) ? 0xffffffff : playerref->getReferencedId());
  
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

uint32_t Planet::getOwner() const{
  return playerref->getReferencedId();
}

void Planet::setOwner(uint32_t no){
  playerref->setReferencedId(no);
  touchModTime();
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
    touchModTime();
}

void Planet::addResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = resources->getResources();
    std::pair<uint32_t, uint32_t> respair = reslist[restype];
    respair.first += amount;
    reslist[restype] = respair;
    resources->setResources(reslist);
    touchModTime();
}

bool Planet::removeResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = resources->getResources();
    if(reslist.find(restype) != reslist.end()){
        if(reslist[restype].first >= amount){
            std::pair<uint32_t, uint32_t> respair = reslist[restype];
            respair.first -= amount;
            reslist[restype] = respair;
            resources->setResources(reslist);
            touchModTime();
            return true;
        }
    }
    return false;
}

}
