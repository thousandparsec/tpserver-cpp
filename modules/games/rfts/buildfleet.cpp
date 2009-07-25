/*  buildfleet
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
#include <cassert>

#include <tpserver/listparameter.h>
#include <tpserver/stringparameter.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/game.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
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
#include "rfts.h"
#include "playerinfo.h"

#include "buildfleet.h"



namespace RFTS_ {

using std::set;
using std::map;
using std::string;
using std::pair;


BuildFleet::BuildFleet() {
   name = "Build Fleet";
   description = "Build a fleet of ships";

   shipList  = (ListParameter*)  addOrderParameter( new ListParameter("Ships","The ships to build", boost::bind( &BuildFleet::generateListOptions, this )));
   fleetName = (StringParameter*)addOrderParameter( new StringParameter( "Name",  "The name of the fleet to build") );

   turns = 1;
}

BuildFleet::~BuildFleet() {

}

Order* BuildFleet::clone() const {
   Order * c = new BuildFleet();
   c->setType(type);
   return c;
}

map<uint32_t, pair< string, uint32_t> > BuildFleet::generateListOptions() {

   map<uint32_t, pair< string, uint32_t> > options;

   Game* game = Game::getGame();

   IGObject *selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   Planet* planetData = dynamic_cast<Planet*>(selectedObj->getObjectBehaviour());

   assert(planetData);

   set<uint32_t> designs = game->getPlayerManager()->getPlayer(
                           planetData->getOwner())->getPlayerView()->getUsableDesigns();

   Game::getGame()->getObjectManager()->doneWithObject(selectedObj->getID());
   DesignStore* ds = Game::getGame()->getDesignStore();
   
   for(set<uint32_t>::iterator i = designs.begin(); i != designs.end(); ++i)
   {
      Design* design = ds->getDesign(*i);
      if(design->getCategoryId() == ds->getCategoryByName("Ships"))
      {
         // limit creation amount by industry and RP

         uint32_t rp = planetData->getCurrentRP(),
                  industry = planetData->getResource("Industry").first,
                  designCost = Rfts::getProductionInfo().getResourceCost(design->getName());

         if(design->getDesignId() == PlayerInfo::getPlayerInfo(planetData->getOwner()).getTransportId())
         {
            options[design->getDesignId()] =
               pair<string,uint32_t>( "Colonist", std::min(planetData->getResource("Colonist").first,
                                                            rp / designCost) );
         }
         else
            options[design->getDesignId()] =
               pair<string, uint32_t>( design->getName(), std::min(industry, rp / designCost) );
      }
   }
  
   return options;
}

void BuildFleet::createFrame(Frame *f, int pos) {
	unsigned turn = Game::getGame()->getTurnNumber() % 3;

	if(turn == 2) // account for move only turn
		turns = 2;
	else
		turns = 1;
	
	Order::createFrame(f, pos);
}

Result BuildFleet::inputFrame(Frame *f, uint32_t playerid) {

   Result r = Order::inputFrame(f, playerid);
   if(!r) return r;


   Game *game = Game::getGame();
   Player* player = game->getPlayerManager()->getPlayer(playerid);
   DesignStore* ds = game->getDesignStore();

   IGObject *selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   Planet* planetData = dynamic_cast<Planet*>(selectedObj->getObjectBehaviour());
   
   IdMap localShipList = shipList->getList();
   uint32_t fleetCostRP = 0;
   
   for(IdMap::iterator i = localShipList.begin(); i != localShipList.end(); ++i)
   {
      uint32_t type = i->first;
      uint32_t numToBuild = i->second;
      

      if(player->getPlayerView()->isUsableDesign(type) && numToBuild >= 0)
      {
         Design* design = ds->getDesign(type);
         uint32_t shipCost = Rfts::getProductionInfo().getResourceCost(design->getName());
         
         fleetCostRP += shipCost * numToBuild;

         // if they're out or RP and trying to build more, reset them to current max
         if(fleetCostRP > planetData->getCurrentRP() )
            numToBuild = (fleetCostRP -  planetData->getCurrentRP()) / shipCost;

         i->second = numToBuild;
         
         ds->designCountsUpdated(design);
      }
      else
      {
         return Failure("The requested design was not valid.");
      }
   }
   if(fleetCostRP == 0 && !localShipList.empty())
      return Failure("To build was empty...");

   shipList->setList(localShipList); // save the list back in case of changes
   
   return Success();
}

bool BuildFleet::doOrder(IGObject *ob)
{
	if(--turns != 0)
		return false;

   if(shipList->getList().size() == 0)
      return true;
   
   Game *game = Game::getGame();

   Planet* planet = dynamic_cast<Planet*>(ob->getObjectBehaviour());
   Player *player = game->getPlayerManager()->getPlayer(planet->getOwner());


   pair<IGObject*, bool> fleetResult =
      createFleet(player, game->getObjectManager()->getObject(ob->getParent()),
                  fleetName->getString(), shipList->getList(), planet);

   IGObject *fleet = fleetResult.first;
   bool complete = fleetResult.second;
   
   //add fleet to universe
   game->getObjectManager()->addObject(fleet);

   player->getPlayerView()->addOwnedObject( fleet->getID() );
   
   // post completion message
   Message * msg = new Message();
   msg->setSubject("Build Fleet order complete");
   string body;

   if(complete)
      body = string("The construction of your new fleet \"") + fleetName->getString() + string("\" is complete.");
   else
      body = string("The construction of your fleet \"") + fleetName->getString() + string("\"\
                    is finished, but not all ships were created due to resource constraints!");
                  
   msg->setBody(PlayerInfo::appAllVictoryPoints(body));
   
   msg->addReference(rst_Action_Order, rsorav_Completion);
   msg->addReference(rst_Object, fleet->getID());
   msg->addReference(rst_Object, ob->getID());
   
   player->postToBoard(msg);
   
   return true;
}


}
