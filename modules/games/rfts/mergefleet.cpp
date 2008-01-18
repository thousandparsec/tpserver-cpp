/*  mergefleet
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

#include <tpserver/frame.h>
#include <tpserver/message.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectorderparameter.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>

#include "fleet.h"
#include "rfts.h"

#include "mergefleet.h"

namespace RFTS_ {

using std::string;
using std::map;
using std::set;

MergeFleet::MergeFleet() {
   name = "Merge Fleet";
   description = "Merge this fleet with another";

   otherFleet = new ObjectOrderParameter();
   otherFleet->setName("Other fleet");
   otherFleet->setDescription("The other fleet to merge with.");
   addOrderParameter(otherFleet);
   
   turns = 1;
}

MergeFleet::~MergeFleet() {

}

Order* MergeFleet::clone() const {
   MergeFleet *mf = new MergeFleet();
   mf->type = this->type;
   return mf;
}

bool MergeFleet::doOrder(IGObject *obj) {
   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();
   
   Fleet *fleetData = dynamic_cast<Fleet*>(obj->getObjectBehaviour());

   IGObject *otherFleetObj = om->getObject(otherFleet->getObjectId());
   Fleet *otherFleetData = dynamic_cast<Fleet*>(otherFleetObj->getObjectBehaviour());

   Player *player = game->getPlayerManager()->getPlayer(fleetData->getOwner());
   
   Message *msg = new Message();
   msg->setSubject("Merge fleet order complete");
   
   if(otherFleetData == NULL || otherFleetData->getOwner() != fleetData->getOwner())
   {
      msg->setBody("Merge failed!<br />Tried to merge with something that wasn't one of your fleets!");
      msg->addReference(rst_Action_Order, rsorav_Canceled);
      msg->addReference(rst_Object, otherFleetObj->getID());
   }
   else
   {
      map<int,int> ships = otherFleetData->getShips();
      for(map<int,int>::iterator i = ships.begin(); i != ships.end(); ++i)
         fleetData->addShips(i->first, i->second);
   
      msg->setBody(string("Merged \"") + obj->getName() + "\" with \"" + otherFleetObj->getName());
      msg->addReference(rst_Action_Order, rsorav_Completion);
      
      //otherFleetObj->removeFromParent(); CHECK
      om->scheduleRemoveObject(otherFleetObj->getID());
   }

   om->doneWithObject(otherFleetObj->getID());

   msg->addReference(rst_Object, obj->getID());
   
   player->postToBoard(msg);

   return true;
}

}
