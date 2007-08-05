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

using std::set;
using std::map;
using std::string;
using std::pair;


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

map<uint32_t, pair< string, uint32_t> > BuildFleet::generateListOptions() {

   map<uint32_t, pair< string, uint32_t> > options;

   Game* game = Game::getGame();

   IGObject *selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());

   set<unsigned int> designs = game->getPlayerManager()->getPlayer(
                           dynamic_cast<OwnedObject*>(selectedObj->getObjectData())->getOwner())->
                          getPlayerView()->getUsableDesigns();

   Game::getGame()->getObjectManager()->doneWithObject(selectedObj->getID());
   DesignStore* ds = Game::getGame()->getDesignStore();
   
   for(set<uint>::iterator i = designs.begin(); i != designs.end(); ++i)
   {
      Design* design = ds->getDesign(*i);
      if(design->getCategoryId() == ds->getCategoryByName("Ships"))
      {
         options[design->getDesignId()] = pair<string, uint32_t>(design->getName(), 100);
      }
   }
  
   return options;
}

Result BuildFleet::inputFrame(Frame *f, unsigned int playerid) {

   Result r = Order::inputFrame(f, playerid);
   if(!r) return r;
   
   Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
   DesignStore* ds = Game::getGame()->getDesignStore();
   
   std::map<uint32_t, uint32_t> fleettype = shipList->getList();
   uint32_t usedshipres = 0, costId = ds->getPropertyByName("RP Cost");
   
   
   for(std::map<uint32_t, uint32_t>::iterator itcurr = fleettype.begin();
      itcurr != fleettype.end(); ++itcurr)
   {
      uint32_t type = itcurr->first;
      uint32_t numToBuild = itcurr->second;

      usedshipres += static_cast<uint32_t>((ds->getDesign(type)->getPropertyValue(costId)) * numToBuild + .05f);

      if(player->getPlayerView()->isUsableDesign(type) && numToBuild >= 0)
      {
         Design* design = ds->getDesign(type);
         design->addUnderConstruction(numToBuild);
         ds->designCountsUpdated(design);
   
      }
      else
      {
         return Failure("The requested design was not valid.");
      }
   }
   if(usedshipres == 0 && !fleettype.empty())
      return Failure("To build was empty...");
   
   resources[1] = usedshipres;
   
   return Success();
}

bool BuildFleet::doOrder(IGObject *ob)
{
  
   Planet* planet = dynamic_cast<Planet*>(ob->getObjectData());
   
   uint32_t usedshipres = resources[1];

   
   int ownerid = planet->getOwner();
   if(ownerid == 0){
         //currently not owned by anyone, just forget about it
         return true;
   }
  
  //planet->addResource(1, 1);
    
  //if(planet->removeResource(1, usedshipres)){
    
   Game *game = Game::getGame();

   IGObject *fleet = game->getObjectManager()->createNewObject();
   fleet->setType(game->getObjectDataManager()->getObjectTypeByName("Fleet"));
   
   //add fleet to star sys
   fleet->addToParent(ob->getParent());
   
   fleet->setName("New Fleet");
   
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
      game->getDesignStore()->designCountsUpdated(design);
   }
   //add fleet to universe
   game->getObjectManager()->addObject(fleet);
   game->getObjectManager()->getObject(ob->getParent())->touchModTime();
   
   // post completion message
   Message * msg = new Message();
   msg->setSubject("Build Fleet order complete");
   msg->setBody(std::string("The construction of your new fleet is complete."));
   msg->addReference(rst_Action_Order, rsorav_Completion);
   msg->addReference(rst_Object, fleet->getID());
   msg->addReference(rst_Object, ob->getID());
   
   Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);
   
   return true;
}


}
