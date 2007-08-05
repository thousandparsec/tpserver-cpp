/*  buildfleet
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

#include <cmath>

#include <tpserver/listparameter.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectdata.h>
#include <tpserver/game.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/message.h>

#include "ownedobject.h"
#include "planet.h"
#include "fleet.h"

#include "buildfleet.h"

namespace RFTS_ {

BuildFleet::BuildFleet() {
   name = "Build Fleet";
   description = "Build a fleet of ships";

   shipList = new ListParameter();
   shipList->setName("Ships");
   shipList->setDescription("The ships to build");
   shipList->setListOptionsCallback(ListOptionCallback(this, &BuildFleet::generateListOptions));
   
   addOrderParameter(shipList);
}

BuildFleet::~BuildFleet() {

}

Order* BuildFleet::clone() const {
   Order * c = new BuildFleet();
   c->setType(type);
   return c;
}

void BuildFleet::createFrame(Frame *f, int pos) {

   turns = 0;
   Order::createFrame(f, pos);
}

std::map<uint32_t, std::pair<std::string, uint32_t> > BuildFleet::generateListOptions(){
  std::map<uint32_t, std::pair<std::string, uint32_t> > options;
  
  std::set<unsigned int> designs = 
      Game::getGame()->getPlayerManager()->getPlayer(
      dynamic_cast<OwnedObject*>(Game::getGame()->getObjectManager()->getObject(
      Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId())
      ->getObjectData())->getOwner())->getPlayerView()->getUsableDesigns();


    Game::getGame()->getObjectManager()->doneWithObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  DesignStore* ds = Game::getGame()->getDesignStore();

  std::set<Design*> usable;
  for(std::set<uint>::iterator itcurr = designs.begin(); itcurr != designs.end(); ++itcurr){
      Design* design = ds->getDesign(*itcurr);
      if(design->getCategoryId() == 1){
          usable.insert(design);
      }
  }

  for(std::set<Design*>::iterator itcurr = usable.begin();
      itcurr != usable.end(); ++itcurr){
    Design * design = (*itcurr);
    options[design->getDesignId()] = std::pair<std::string, uint32_t>(design->getName(), 100);
  }
  
  return options;
}

Result BuildFleet::inputFrame(Frame *f, unsigned int playerid)
{
  Result r = Order::inputFrame(f, playerid);
  if(!r) return r;
  
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
  DesignStore* ds = Game::getGame()->getDesignStore();
  
  std::map<uint32_t, uint32_t> fleettype = shipList->getList();
  uint32_t usedshipres = 0;
  
  for(std::map<uint32_t, uint32_t>::iterator itcurr = fleettype.begin();
     itcurr != fleettype.end(); ++itcurr){
    uint32_t type = itcurr->first;
    uint32_t number = itcurr->second; // number to build
    
    if(player->getPlayerView()->isUsableDesign(type) && number >= 0){
      
      Design* design = ds->getDesign(type);
        design->addUnderConstruction(number);
        ds->designCountsUpdated(design);

    }else{
      return Failure("The requested design was not valid.");
    }
  }
  if(usedshipres == 0 && !fleettype.empty()){
    return Failure("To build was empty...");
  }
  
  resources[1] = usedshipres;

  return Success();
}

bool BuildFleet::doOrder(IGObject *ob)
{
  
  Planet* planet = dynamic_cast<Planet*>(ob->getObjectData());

  uint32_t usedshipres = resources[1];
  
  if(usedshipres == 0)
    return true;

  int ownerid = planet->getOwner();
  if(ownerid == 0){
      //currently not owned by anyone, just forget about it
      return true;
  }
  
  planet->addResource(1, 1);
    
  if(planet->removeResource(1, usedshipres)){
    //create fleet
    
    IGObject *fleet = Game::getGame()->getObjectManager()->createNewObject();
    fleet->setType(Game::getGame()->getObjectDataManager()->getObjectTypeByName("Fleet"));
    
    //add fleet to container
    fleet->addToParent(ob->getID());
    
    //fleet->setName(fleetname->getString().c_str());
    
    Fleet * fleetData = dynamic_cast<Fleet*>(fleet->getObjectData());
    
    fleetData->setSize(2);
    fleetData->setOwner(ownerid); // set ownerid
    fleetData->setPosition(planet->getPosition());
    fleetData->setVelocity(Vector3d(0LL, 0ll, 0ll));
    
    OrderQueue *fleetoq = new OrderQueue();
    fleetoq->setObjectId(fleet->getID());
    fleetoq->addOwner(ownerid);
    Game::getGame()->getOrderManager()->addOrderQueue(fleetoq);
    OrderQueueObjectParam* oqop = dynamic_cast<OrderQueueObjectParam*>(
                                 fleet->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(fleetoq->getQueueId());
    fleetData->setDefaultOrderTypes();
    
    //set ship type
    std::map<uint32_t,uint32_t> fleettype = shipList->getList();
    for(std::map<uint32_t,uint32_t>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
      fleetData->addShips(itcurr->first, itcurr->second);
        Design* design = Game::getGame()->getDesignStore()->getDesign(itcurr->first);
        design->addComplete(itcurr->second);
        Game::getGame()->getDesignStore()->designCountsUpdated(design);
    }
    //add fleet to universe
    Game::getGame()->getObjectManager()->addObject(fleet);

    Message * msg = new Message();
    msg->setSubject("Build Fleet order complete");
    msg->setBody(std::string("The construction of your new fleet is complete."));
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, fleet->getID());
    msg->addReference(rst_Object, ob->getID());
 
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);

    return true;
  }
  return false;
}


}
