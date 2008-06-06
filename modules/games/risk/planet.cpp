/*  Planet
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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

#include <cmath>
#include <map>

#include <tpserver/frame.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/prng.h>
#include <tpserver/object.h>
#include <tpserver/message.h>
#include <tpserver/playermanager.h>
#include <tpserver/player.h>
#include <tpserver/logging.h>
#include <tpserver/objectmanager.h>

#include "containertypes.h"
#include "risk.h"
#include "planet.h"


namespace RiskRuleset {

using std::string;
using std::pair;
using std::map;
using std::set;

PlanetType::PlanetType() : StaticObjectType(){
   nametype = "Planet";
   typedesc = "A planet object";

   ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
   group->setName("Ownership");
   group->setDescription("The ownership of this object");
   group->addParameter(obpT_Reference, "Owner", "The owner of this object");
   addParameterGroupDesc(group);               //(2,1)

   group = new ObjectParameterGroupDesc();
   group->setName("Resources");
   group->setDescription("The planets stats");
   group->addParameter(obpT_Resource_List, "Resource List", "The for this planet");
   addParameterGroupDesc(group);               //(3,1)

   group = new ObjectParameterGroupDesc();
   group->setName("Orders");
   group->setDescription("The order queues of the planet");
   group->addParameter(obpT_Order_Queue, "Order Queue", "The queue of orders for this planet");
   addParameterGroupDesc(group);               //(4,1)
}

ObjectBehaviour* PlanetType::createObjectBehaviour() const{
   return new Planet();
}

Planet::Planet() : StaticObject() {
}

Planet::~Planet() {

}

void Planet::setOrderTypes() {
   OrderManager *om = Game::getGame()->getOrderManager();

   std::set<uint32_t> allowedlist;

   allowedlist.insert(om->getOrderTypeByName("Colonize"));
   allowedlist.insert(om->getOrderTypeByName("Move"));
   allowedlist.insert(om->getOrderTypeByName("Reinforce"));

   ((OrderQueueObjectParam*)(obj->getParameter(4,1)))->setAllowedOrders(allowedlist);
}

void Planet::doOnceATurn() {
   Logger::getLogger()->debug("starting doOnceATurn for Planet");
   Risk* risk = dynamic_cast<Risk*>(Game::getGame()->getRuleset());

   uint32_t owner = getOwner();                                      //Get ID of planet owner
   if (owner > 0) { //ensure the owner is real
      uint32_t reinforcements = risk->getPlayerReinforcements(owner);   //Get players max reinforcements
      uint32_t current = getResource("Army").first;                     //Get planets current resources
      //CHECK: if the first element of the std::pair is the number of armies on surface

      //Update the display of resources to show new army and max count (max is total availible reinforcements)
      setResource("Army", current, reinforcements);  
   }
   setOrderTypes();

   obj->touchModTime();
}

int Planet::getContainerType() {
   return ContainerTypes_::Planet;
}

void Planet::packExtraData(Frame * frame){
   ReferenceObjectParam* playerref = ((ReferenceObjectParam*)(obj->getParameter(2,1)));
   frame->packInt((playerref->getReferencedId() == 0) ? 0xffffffff : playerref->getReferencedId());

   map<uint32_t, pair<uint32_t, uint32_t> > reslist = ((ResourceListObjectParam*)(obj->getParameter(3,1)))->getResources();
   frame->packInt(reslist.size());
   for(map<uint32_t, pair<uint32_t, uint32_t> >::iterator itcurr = reslist.begin();
   itcurr != reslist.end(); ++itcurr){
      frame->packInt(itcurr->first);
      frame->packInt(itcurr->second.first);
      frame->packInt(itcurr->second.second);
      frame->packInt(0);
   }
}

uint32_t Planet::getOwner() const{
   return ((ReferenceObjectParam*)(obj->getParameter(2,1)))->getReferencedId();
}

void Planet::setOwner(uint32_t no){
   ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferencedId(no);
   Game::getGame()->getOrderManager()->getOrderQueue(((OrderQueueObjectParam*)(obj->getParameter(4,1)))->getQueueId())->addOwner(no);

   obj->touchModTime();
}

void Planet::setDefaultResources() {
//set resource army to 0 total with a max "minable" of 0
//Minable units are used to indicate availible reinforcements - This is updated in doOnceATurn
   setResource("Army", 0, 0);
}

const pair<uint32_t, uint32_t> Planet::getResource(uint32_t resTypeId) const {
   return ResourceListParam((obj->getParameter(3,1))).getResource(resTypeId);
}

const pair<uint32_t, uint32_t> Planet::getResource(const string& resTypeName) const {
   return RiskRuleset::getResource(ResourceListParam((obj->getParameter(3,1))),
      resTypeName);
}

void Planet::setResource(uint32_t resTypeId, uint32_t currVal, uint32_t maxVal) {
   ResourceListParam(obj->getParameter(3,1)).setResource(resTypeId, currVal, maxVal);
}

void Planet::setResource(const string& resType, uint32_t currVal, uint32_t maxVal) {
   ResourceListParam res = ResourceListParam(obj->getParameter(3,1));
   RiskRuleset::setResource( res, resType, currVal, maxVal);
}

const map<uint32_t, pair<uint32_t, uint32_t> > Planet::getResources() const{
   return dynamic_cast<ResourceListObjectParam*>(obj->getParameter(3,1))->getResources();
}

void Planet::addResource(uint32_t resTypeId, uint32_t amount){
   ResourceListParam((obj->getParameter(3,1))).addResource(resTypeId, amount);
   obj->touchModTime();
}

void Planet::addResource(const string& resType, uint32_t amount){
   addResource( getTypeId(resType), amount );
   obj->touchModTime();
}

bool Planet::removeResource(uint32_t resTypeId, uint32_t amount){
   ResourceListObjectParam& res = *dynamic_cast<ResourceListObjectParam*>(obj->getParameter(3,1));
   std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = res.getResources();

   if(reslist.find(resTypeId) != reslist.end()){
      obj->touchModTime();
      if(reslist[resTypeId].first >= amount){
         std::pair<uint32_t, uint32_t> respair = reslist[resTypeId];
         respair.first -= amount;
         reslist[resTypeId] = respair;
         res.setResources(reslist);
      } else {
         reslist[resTypeId].first = 0;
         res.setResources(reslist);
      }
      return true;
   }
   return false;
}

bool Planet::removeResource(const string& resTypeName, uint32_t amount){

   return removeResource( getTypeId(resTypeName), amount);
}



void Planet::setupObject(){
   setSize(2);
   ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferenceType(rst_Player);
}

}
