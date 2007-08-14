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
#include <cassert>

#include <tpserver/listparameter.h>
#include <tpserver/stringparameter.h>
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
#include "productioninfo.h"
#include "buildfleet.h"

#include "rfts.h"

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

   fleetName = new StringParameter();
   fleetName->setName("Name");
   fleetName->setDescription("The name of the fleet to build");
   
   addOrderParameter(fleetName);
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
   Planet* planetData = dynamic_cast<Planet*>(selectedObj->getObjectData());

   assert(planetData);

   set<unsigned int> designs = game->getPlayerManager()->getPlayer(
                           planetData->getOwner())->getPlayerView()->getUsableDesigns();

   Game::getGame()->getObjectManager()->doneWithObject(selectedObj->getID());
   DesignStore* ds = Game::getGame()->getDesignStore();
   
   for(set<uint>::iterator i = designs.begin(); i != designs.end(); ++i)
   {
      Design* design = ds->getDesign(*i);
      if(design->getCategoryId() == ds->getCategoryByName("Ships"))
      {
         // limit creation amount by industry and RP

         uint32_t rp = planetData->getCurrentRP(), industry = planetData->getResource("Industry").first,
                  designCost = Rfts::getProductionInfo().getResourceCost(design->getName());
 
         options[design->getDesignId()] =
            pair<string, uint32_t>( design->getName(), std::min(industry, rp / designCost) );
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
   uint32_t fleetCostRP = 0;
   
   
   for(std::map<uint32_t, uint32_t>::iterator itcurr = fleettype.begin();
      itcurr != fleettype.end(); ++itcurr)
   {
      uint32_t type = itcurr->first;
      uint32_t numToBuild = itcurr->second;
      

      if(player->getPlayerView()->isUsableDesign(type) && numToBuild >= 0)
      {
         Design* design = ds->getDesign(type);
         uint32_t shipCost = Rfts::getProductionInfo().getResourceCost(design->getName());
         
         fleetCostRP += shipCost * numToBuild;
         ds->designCountsUpdated(design);
      }
      else
      {
         return Failure("The requested design was not valid.");
      }
   }
   if(fleetCostRP == 0 && !fleettype.empty())
      return Failure("To build was empty...");
   
   resources[1] = fleetCostRP;
   
   return Success();
}

bool BuildFleet::doOrder(IGObject *ob)
{
   Game *game = Game::getGame();

   Planet* planet = dynamic_cast<Planet*>(ob->getObjectData());
   Player *player = game->getPlayerManager()->getPlayer(planet->getOwner());


   pair<IGObject*, uint32_t> fleetResult =
      createFleet(player, game->getObjectManager()->getObject(ob->getParent()),
                  fleetName->getString(), shipList->getList(), planet->getCurrentRP());

   IGObject *fleet = fleetResult.first;
   uint32_t usedRP = fleetResult.second;
   
   //add fleet to universe
   game->getObjectManager()->addObject(fleet);

   planet->removeResource("Resource Point", usedRP);

   player->getPlayerView()->addVisibleObject( fleet->getID() );
   
   // post completion message
   Message * msg = new Message();
   msg->setSubject("Build Fleet order complete");

   if(usedRP < planet->getCurrentRP())
      msg->setBody(string("The construction of your new fleet \"") + fleetName->getString() +
                  string("\" is complete."));
   else
      msg->setBody(string("The construction of your fleet \"") + fleetName->getString() + string("\"\
                    is finished, but not all ships were created due to resource constraints!"));
                  
   
   msg->addReference(rst_Action_Order, rsorav_Completion);
   msg->addReference(rst_Object, fleet->getID());
   msg->addReference(rst_Object, ob->getID());
   
   player->postToBoard(msg);
   
   return true;
}


}
