/*  splitfleet
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

#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/message.h>
#include <tpserver/logging.h>

#include <tpserver/listparameter.h>
#include <tpserver/orderqueue.h>

#include "fleet.h"
#include "staticobject.h"
 
#include "splitfleet.h"

namespace RFTS_ {

using std::map;
using std::string;
using std::pair;

SplitFleet::SplitFleet() {
   name = "Split Fleet";
   description = "Split off a new fleet.";
   
   shipList = new ListParameter();
   shipList->setName("ships");
   shipList->setDescription("The ships to make a new fleet out of");
   shipList->setListOptionsCallback(ListOptionCallback(this, &SplitFleet::generateListOptions));
   addOrderParameter(shipList);
   
   turns = 1;
}

SplitFleet::~SplitFleet() {

}


Order* SplitFleet::clone() const {
   SplitFleet *sf = new SplitFleet();
   sf->type = this->type;
   return sf;
}

bool SplitFleet::doOrder(IGObject *obj) {

   Game * game = Game::getGame();
   ObjectManager *om = game->getObjectManager();

   Fleet *fleetData = dynamic_cast<Fleet*>(obj->getObjectBehaviour());
   Player *player = game->getPlayerManager()->getPlayer(fleetData->getOwner());
   IGObject *starSys = om->getObject(obj->getParent());
   
   IGObject * newFleet = createFleet(player, starSys, "New fleet", shipList->getList());
   Fleet *newFleetData = dynamic_cast<Fleet*>(newFleet->getObjectBehaviour());

   bool failed = false;

   Message * msg = new Message();
   msg->setSubject("Split Fleet order complete");
   msg->addReference(rst_Object, obj->getID());
   
   map<uint32_t, uint32_t> ships = shipList->getList();
   for(map<uint32_t, uint32_t>::iterator i = ships.begin(); i != ships.end(); ++i)
   {
      if(!fleetData->removeShips(i->first, i->second))
         failed = true;
   }

   
   if(fleetData->totalShips() == 0 || failed)
   {
      // whole fleet moved, put it back
      Logger::getLogger()->debug("Whole fleet split, putting it back");
      for(std::map<uint32_t, uint32_t>::iterator i = ships.begin(); i != ships.end(); ++i)
      {
         fleetData->addShips(i->first, i->second);
      }
      om->discardNewObject(newFleet);
      newFleet->removeFromParent();
      msg->setBody("Fleet not split, not enough ships");
      msg->addReference(rst_Action_Order, rsorav_Incompatible);
   }
   else if(newFleetData->totalShips() == 0)
   {
      Logger::getLogger()->debug("Split fleet doesn't have any ships, not creating new fleet");
      om->discardNewObject(newFleet);
      newFleet->removeFromParent();
      msg->setBody("Fleet not split, not enough ships");
      msg->addReference(rst_Action_Order, rsorav_Incompatible);
   }
   else
   {
      // add fleet to game universe
      Logger::getLogger()->debug("Split fleet successfully");
      msg->setBody("Split fleet complete");
      msg->addReference(rst_Object, newFleet->getID());
      msg->addReference(rst_Action_Order, rsorav_Completion);
      
      newFleet->addToParent(obj->getParent());
      om->addObject(newFleet);
      player->getPlayerView()->addOwnedObject(newFleet->getID());
   }

   
   player->postToBoard(msg);
   
   return true;
}

map<uint32_t, pair<string, uint32_t> > SplitFleet::generateListOptions() {
   std::map<uint32_t, std::pair<std::string, uint32_t> > options;

   Game* game = Game::getGame();
   ObjectManager* om = game->getObjectManager();
   OrderManager *orm = game->getOrderManager();

   IGObject* fleet = om->getObject(orm->getOrderQueue(orderqueueid)->getObjectId());
   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());

   map<int, int> ships = fleetData->getShips();
   
   om->doneWithObject(fleet->getID());

   DesignStore* ds = game->getDesignStore();

   for(map<int, int>::const_iterator i = ships.begin(); i != ships.end(); ++i)
      options[i->first] = pair<string, uint32_t>(ds->getDesign(i->first)->getName(), i->second);

   return options;
}

}
