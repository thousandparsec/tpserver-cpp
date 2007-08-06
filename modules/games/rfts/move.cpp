/*  move
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

#include <cassert>

#include <tpserver/frame.h>
#include <tpserver/objectorderparameter.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectdata.h>
#include <tpserver/objectmanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/message.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>

#include "ownedobject.h"
#include "fleet.h"
#include "planet.h"

#include "move.h"

namespace RFTS_ {

using std::string;

Move::Move() : Order() {
   name = "Move";
   description = "Move to a given planet";
   
   planet = new ObjectOrderParameter();
   planet->setName("planet");
   planet->setDescription("The planet to move to");
   addOrderParameter(planet);
}

Move::~Move() {

}

Order* Move::clone() const {
   Move* o = new Move();
   o->type = type;
   return o;
}

void Move::createFrame(Frame *f, int pos) {

   turns = -1;

   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();

   IGObject *fleet = om->getObject(game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());

   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectData());
   StaticObject* planetData = dynamic_cast<StaticObject*>(om->getObject(planet->getObjectId())->getObjectData());

   if(planetData != NULL && fleetData != NULL)
      // need to calculate speed of fastest ship in fleet and do real calc below
      turns = static_cast<uint32_t>(fleetData->getPosition().getDistanceSq(planetData->getPosition()) /
                                       (9000. * 9000.) + .5);

   Order::createFrame(f, pos);
}

Result Move::inputFrame(Frame *f, uint32_t playerid) {
   return Order::inputFrame(f, playerid);
}

bool Move::doOrder(IGObject * obj) {
   turns--;

   if(turns <= 0)
   {
      ObjectManager* om = Game::getGame()->getObjectManager();
      Fleet* fleetData = dynamic_cast<Fleet*>(obj->getObjectData());

      IGObject *oldPlanet = om->getObject(obj->getParent());
      IGObject *newPlanet = om->getObject(planet->getObjectId());

      obj->removeFromParent();
      obj->setParent(planet->getObjectId());
      fleetData->setPosition(dynamic_cast<StaticObject*>(newPlanet->getObjectData())->getPosition());

      oldPlanet->touchModTime();
      newPlanet->touchModTime();
      obj->touchModTime();
   
      // post completion message
      Message * msg = new Message();
      msg->setSubject("Move fleet order complete");
      msg->setBody(string("You're fleet has arrived at ") + om->getObject(planet->getObjectId())->getName() + ".");
      msg->addReference(rst_Action_Order, rsorav_Completion);
      msg->addReference(rst_Object, obj->getID());
      msg->addReference(rst_Object, planet->getObjectId());
      
      Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner())->postToBoard(msg);

      return true;
   }
   else
      return false;
}

}
