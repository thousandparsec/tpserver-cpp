/*  Planet objects
 *
 *  Copyright (C) 2003-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/order.h>
#include <tpserver/ordermanager.h>
#include <tpserver/game.h>
#include <tpserver/resourcelistobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/objectparametergroupdesc.h>

#include "planet.h"

PlanetType::PlanetType():OwnedObjectType("Planet", "A planet object") {
  ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc("Resources","The planets resources");
  group->addParameter(obpT_Resource_List, "Resource List", "The resource list of the resources the planet has available");
}

PlanetType::~PlanetType(){
}

ObjectBehaviour* PlanetType::createObjectBehaviour() const{
  return new Planet();
}

const uint32_t Planet::RESGRPID = 5;
const uint32_t Planet::RESPARAMID = 1;

Planet::Planet(){
}

Planet::~Planet(){
}

void Planet::setDefaultOrderTypes(){
  OrderManager * om = Game::getGame()->getOrderManager();
  std::set<uint32_t> allowedlist;
  allowedlist.insert(om->getOrderTypeByName("Build Fleet"));
  allowedlist.insert(om->getOrderTypeByName("No Operation"));
  ((OrderQueueObjectParam*)(obj->getParameter(ORDERGRPID,ORDERQPARAMID)))->setAllowedOrders(allowedlist);
}

void Planet::packExtraData(OutputFrame::Ptr frame){
  OwnedObject::packExtraData(frame);
  
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->getResources();
    frame->packInt(reslist.size());
    for(std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator itcurr = reslist.begin();
            itcurr != reslist.end(); ++itcurr){
        frame->packInt(itcurr->first);
        frame->packInt(itcurr->second.first);
        frame->packInt(itcurr->second.second);
        frame->packInt(0);
    }

}

void Planet::doOnceATurn()
{

}

int Planet::getContainerType(){
  return 2;
}



std::map<uint32_t, std::pair<uint32_t, uint32_t> > Planet::getResources(){
    return ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->getResources();
}

uint32_t Planet::getResource(uint32_t restype) const{
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->getResources();
  if(reslist.find(restype) != reslist.end()){
    return reslist.find(restype)->second.first;
  }
  return 0;
}

void Planet::setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress){
    ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->setResources(ress);
    obj->touchModTime();
}

void Planet::addResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->getResources();
    std::pair<uint32_t, uint32_t> respair = reslist[restype];
    respair.first += amount;
    reslist[restype] = respair;
    ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->setResources(reslist);
    obj->touchModTime();
}

bool Planet::removeResource(uint32_t restype, uint32_t amount){
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->getResources();
    if(reslist.find(restype) != reslist.end()){
        if(reslist[restype].first >= amount){
            std::pair<uint32_t, uint32_t> respair = reslist[restype];
            respair.first -= amount;
            reslist[restype] = respair;
            ((ResourceListObjectParam*)(obj->getParameter(RESGRPID,RESPARAMID)))->setResources(reslist);
            obj->touchModTime();
            return true;
        }
    }
    return false;
}


IGObject::Ptr Planet::createObject(IGObject::Ptr parent, std::string name, Vector3d v, uint32_t size, std::string media) {

  Game* game = Game::getGame();
  ObjectManager* obman = game->getObjectManager();
  ObjectTypeManager* otypeman = game->getObjectTypeManager();

  EmptyObject* theparent = (EmptyObject*)(parent->getObjectBehaviour());

  IGObject::Ptr planet = game->getObjectManager()->createNewObject();
  otypeman->setupObject(planet, otypeman->getObjectTypeByName("Planet"));

  planet->setName(name);
  planet->addToParent(parent->getID());

  Planet* theplanet = (Planet*)(planet->getObjectBehaviour());
  theplanet->setSize(2);
  theplanet->setPosition(theparent->getPosition() + v);
  uint32_t queueid = game->getOrderManager()->addOrderQueue(planet->getID(), 0);

  OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(planet->getParameterByType(obpT_Order_Queue));
  oqop->setQueueId(queueid);

  theplanet->setDefaultOrderTypes();
  theplanet->setIcon("common/object-icons/planet");
  theplanet->setMedia(std::string("common-2d/foreign/freeorion/planet-small/animation/") + media);

  obman->addObject(planet);

  return planet;
}
