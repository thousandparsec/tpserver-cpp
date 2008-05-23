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

//modified these include - jphr
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
  addParameterGroupDesc(group);
  
  //Need to find out how to make # armies a property
  /*
  group = new ObjectParameterGroupDesc();
  group->setName("Armies");
  group->setDescription("The planets stats");
  group->addParameter(obpT_Armies, "Number of Armies", "The number of armies for this planet.");
  addParameterGroupDesc(group);
  */
  
  group = new ObjectParameterGroupDesc();
  group->setName("Orders");
  group->setDescription("The order queues of the planet");
  group->addParameter(obpT_Order_Queue, "Order Queue", "The queue of orders for this planet");
  addParameterGroupDesc(group);
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

   //modified here - jphr
	allowedlist.insert(om->getOrderTypeByName("Colonize"));
	allowedlist.insert(om->getOrderTypeByName("Move"));
	allowedlist.insert(om->getOrderTypeByName("Reinforce"));
      
   ((OrderQueueObjectParam*)(obj->getParameter(4,1)))->setAllowedOrders(allowedlist);
}

void Planet::doOnceATurn() {
  
    //TODO: insert logic (if any)
  
   /* NOT APPLICABLE TO RISK AT ALL
   unsigned turn = Game::getGame()->getTurnNumber() % 3;
   

   if(getOwner() != 0)
   {
      if(turn == 0) // next turn is 0 - do RP production
      {
         // calc RP for next turn
         calcRP();
      }
      else if(turn == 1) // just did a prod. turn
      {
			calcPopulation();
         upgradePdbs();
      }
		
		this->setResource("Ship Technology", 0, PlayerInfo::getPlayerInfo(getOwner()).getShipTechPoints());
   }
	  */
   setOrderTypes();

   obj->touchModTime();
}

int Planet::getContainerType() {
   return ContainerTypes_::Planet;
}

void Planet::packExtraData(Frame * frame){
  ReferenceObjectParam* playerref = ((ReferenceObjectParam*)(obj->getParameter(2,1)));
  frame->packInt((playerref->getReferencedId() == 0) ? 0xffffffff : playerref->getReferencedId());
  
  /* No need to pack resource list that doesn't exist - may not need to pack any extra data
  map<uint32_t, pair<uint32_t, uint32_t> > reslist = ((ResourceListObjectParam*)(obj->getParameter(3,1)))->getResources();
    frame->packInt(reslist.size());
    for(map<uint32_t, pair<uint32_t, uint32_t> >::iterator itcurr = reslist.begin();
            itcurr != reslist.end(); ++itcurr){
        frame->packInt(itcurr->first);
        frame->packInt(itcurr->second.first);
        frame->packInt(itcurr->second.second);
        frame->packInt(0);
    }
    */
}

uint32_t Planet::getOwner() const{
  return ((ReferenceObjectParam*)(obj->getParameter(2,1)))->getReferencedId();
}

void Planet::setOwner(uint32_t no){
  ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferencedId(no);
  Game::getGame()->getOrderManager()->getOrderQueue(((OrderQueueObjectParam*)(obj->getParameter(4,1)))->getQueueId())->addOwner(no);

  obj->touchModTime();
}

void Planet::setupObject(){
  setSize(2);
  ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferenceType(rst_Player);
}

}
