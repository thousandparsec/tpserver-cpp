/*  Planet
 *
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

#include "resourcelistparam.h"
#include "containertypes.h"
#include "rfts.h"
#include "productioninfo.h"
#include "playerinfo.h"

#include "planet.h"


namespace RFTS_ {

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
  
  group = new ObjectParameterGroupDesc();
  group->setName("Resources");
  group->setDescription("The planets stats");
  group->addParameter(obpT_Resource_List, "Resource List", "The for this planet");
  addParameterGroupDesc(group);
  
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

	allowedlist.insert(om->getOrderTypeByName("Produce"));
	allowedlist.insert(om->getOrderTypeByName("Build Fleet"));
      
   ((OrderQueueObjectParam*)(obj->getParameter(4,1)))->setAllowedOrders(allowedlist);
}

void Planet::doOnceATurn() {

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
	
   setOrderTypes();

   obj->touchModTime();
}

void Planet::calcRP() {
	pair<uint32_t,uint32_t> popn = this->getResource("Population");
	this->setResource("Resource Point",
         ( std::min(popn.first, popn.second) * 2) +
			(this->getResource("Industry").first * this->getResource("Social Environment").first)
                                             / 16);
}

void Planet::calcPopulation() {

   Random *rand = Game::getGame()->getRandom();

	ResourceListParam res = ResourceListParam(obj->getParameter(3,1));
	int newPop = getResource("Population").first;
   const pair<uint32_t,uint32_t> planetary = getResource("Planetary Environment");
   pair<uint32_t,uint32_t> social = RFTS_::getResource(res, "Social Environment");

            // social + midpoint of difference of percentage of planetary and social
   social.first = static_cast<uint32_t>( social.first + .125 *
                   ((static_cast<double>(planetary.first) / planetary.second) -
                    (static_cast<double>(social.first) / social.second))) ;

	uint32_t popMaint = RFTS_::getResource(res, "Population Maintenance").first;

   // add in a lil' randomness
   popMaint += static_cast<uint32_t>(popMaint * (rand->getInRange(-50,125) / 1000.) );

   if(newPop != 0 && static_cast<int>(popMaint) < newPop)
      newPop -= (newPop - popMaint ) / 3;
   
      // social < 40 => pop goes down, else goes up
   newPop += static_cast<int>( newPop * ((social.first - 40) / 9.) );

   if(newPop < 0)
	{
      newPop = 0;
    Message::Ptr msg( new Message() );
		msg->setSubject("Planet population dead!");
		msg->setBody( "The population on planet, " + obj->getName() + ", has completely died.<br/>\
				Recolonisation and social environment buffing is recommended." );
		msg->addReference(rst_Object, obj->getID());
		Game::getGame()->getPlayerManager()->getPlayer(getOwner())->postToBoard(msg);
						
		// no population, no the owner
		// also stops message being repeated every turn
		setOwner(0);
	}

   RFTS_::setResource(res, "Population", static_cast<uint32_t>(newPop) );
	RFTS_::setResource(res, "Social Environment", social.first);
	RFTS_::setResource(res, "Population Maintenance", 0); // use up pop maint

}

void Planet::upgradePdbs() {
   if(PlayerInfo::getPlayerInfo(getOwner()).upgradePdbs())
   {
		ResourceListParam res = ResourceListParam(obj->getParameter(3,1));
		
      const char techLevel = PlayerInfo::getPlayerInfo(getOwner()).getShipTechLevel();
      uint32_t totalPdbs = 0;

      // just in case they skip a tech level, make sure we upgrade any/all pdbs
      for(char oldTech = static_cast<char>(techLevel - 1); oldTech >= '1'; oldTech--)
      {
			uint32_t oldPdbs = RFTS_::getResource(res, string("PDB") + oldTech).first;
			RFTS_::setResource(res, string("PDB") + oldTech, 0);
         totalPdbs += oldPdbs;
      }

		RFTS_::setResource(res, string("PDB") + techLevel, totalPdbs);
   }
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

   Random* rand = Game::getGame()->getRandom();
   const ProductionInfo& po = Rfts::getProductionInfo();

   setSize(rand->getInRange(1,ProductionInfo::TOTAL_PLANET_TYPES - 1));
   
   ProductionInfo::PlanetType planetType = static_cast<ProductionInfo::PlanetType>(getSize());
   
   setResource("Resource Point", 0, 0);
   setResource("Industry", 0,  po.getRandResourceVal("Industry", planetType));
   setResource("Population", 0, po.getRandResourceVal("Population", planetType));
   setResource("Social Environment", 0, po.getRandResourceVal("Social Environment", planetType));
   setResource("Planetary Environment", 0, po.getRandResourceVal("Planetary Environment", planetType));

}

const uint32_t Planet::getCurrentRP() const {
	return this->getResource("Resource Point").first;
}

const pair<uint32_t, uint32_t> Planet::getResource(uint32_t resTypeId) const {
	return ResourceListParam((obj->getParameter(3,1))).getResource(resTypeId);
}

const pair<uint32_t, uint32_t> Planet::getResource(const string& resTypeName) const {
	return RFTS_::getResource(ResourceListParam((obj->getParameter(3,1))),
										 resTypeName);
}

void Planet::setResource(uint32_t resTypeId, uint32_t currVal, uint32_t maxVal) {
	ResourceListParam(obj->getParameter(3,1)).setResource(resTypeId, currVal, maxVal);
}

void Planet::setResource(const string& resType, uint32_t currVal, uint32_t maxVal) {
	ResourceListParam res = ResourceListParam(obj->getParameter(3,1));
	RFTS_::setResource( res, resType, currVal, maxVal);
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
