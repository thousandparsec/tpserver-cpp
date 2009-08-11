/*  move
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

#include <cassert>

#include <tpserver/orderparameters.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/message.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/logging.h>

#include "ownedobject.h"
#include "fleet.h"
#include "planet.h"

#include "map.h"

#include "move.h"

namespace RFTS_ {

using std::string;

Move::Move() : Order() {
   name = "Move";
   description = "Move to a given planet";
   
   starSys = (ObjectOrderParameter*)addOrderParameter( new ObjectOrderParameter( "Star System", "The star system to move to") );
}

Move::~Move() {

}

Order* Move::clone() const {
   Move* o = new Move();
   o->type = type;
   return o;
}

void Move::createFrame(OutputFrame *f, int pos) {
   Order::createFrame(f, pos);
}

void Move::inputFrame(InputFrame *f, uint32_t playerid) {
   Order::inputFrame(f, playerid);

   turns = 0;
      
   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();
   ObjectTypeManager *odm = game->getObjectTypeManager();
   

   IGObject::Ptr fleet = om->getObject(game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());

   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());
   assert(fleetData != NULL);

   IGObject::Ptr starSysObj = om->getObject(starSys->getObjectId());

   // if they chose a planet, set to the owning star sys
   if(starSysObj->getType() == odm->getObjectTypeByName("Planet"))
   {
      starSys->setObjectId(starSysObj->getParent());
      Logger::getLogger()->debug("Player trying to move to planet, setting to planet's star sys");
   }
   // if they're just crazy, reset to current position
   else if(starSysObj->getType() != odm->getObjectTypeByName("Star System"))
   {
      starSys->setObjectId(fleet->getParent());
      Logger::getLogger()->debug("Player made illogical move order, resetting move to current pos");
   }

   StaticObject* starSysData = dynamic_cast<StaticObject*>(starSysObj->getObjectBehaviour());
   
   turns = static_cast<uint32_t>(getWrappingUnitDist(*fleetData, *starSysData) / fleetData->getSpeed()  + .5);
   
   om->doneWithObject(fleet->getID());
}

bool Move::doOrder(IGObject::Ptr obj) {
   turns--;

   if(obj->getParent() != 0)
   {
      obj->removeFromParent();
      obj->addToParent(0);
   }


   if(turns <= 0)
   {
      ObjectManager* om = Game::getGame()->getObjectManager();
      Fleet* fleetData = dynamic_cast<Fleet*>(obj->getObjectBehaviour());


      IGObject::Ptr newStarSys = om->getObject(starSys->getObjectId());
      obj->removeFromParent();
      obj->addToParent(starSys->getObjectId());
      fleetData->setUnitPos(dynamic_cast<StaticObject*>(newStarSys->getObjectBehaviour())->getUnitPos());

      exploreStarSys(obj);
      om->doneWithObject(newStarSys->getID());

      Player::Ptr player = Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner());

      // post completion message
      Message::Ptr msg( new Message() );
      msg->setSubject("Move fleet order complete");
      msg->setBody(string("Your fleet \"" + obj->getName() + "\" has arrived and is in orbit around ")
          + newStarSys->getName() + ".");
      msg->addReference(rst_Action_Order, rsorav_Completion);
      msg->addReference(rst_Object, starSys->getObjectId());
      msg->addReference(rst_Object, obj->getID());
      
      player->postToBoard(msg);
      
      

      return true;
   }
   else
      return false;
}

}
