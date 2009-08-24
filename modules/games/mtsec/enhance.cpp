/*  Enhance object for Enhance orders
 *
 *  Copyright (C) 2009 Alan P. Laudicina and the Thousand Parsec Project
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <math.h>
#include <iostream>

#include <tpserver/result.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/vector3d.h>
#include "fleet.h"
#include <tpserver/message.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/playermanager.h>
#include <tpserver/timeparameter.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/resourcelistobjectparam.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>

#include "planet.h"

#include "enhance.h"

#define MAX(x,y) (x<y) ? (y) : (x)
#define MIN(x,y) (x<y) ? (x) : (y)

namespace MTSecRuleset {

Enhance::Enhance() : Order()
{
  name = "Enhance";
  description = "Enhance your Production";

  points = new TimeParameter();
  points->setName("Points");
  points->setMax(100);
  points->setDescription("The number of points you want to enhance with.");
  addOrderParameter(points);

  turns = 1;

}

Enhance::~Enhance(){
}

bool Enhance::doOrder(IGObject *ob)
{
  Logger::getLogger()->debug("Entering Enhance::doOrder");

  Planet* planet = static_cast<Planet*>(ob->getObjectBehaviour());

  int ownerid = planet->getOwner();
  if(ownerid == 0){
      Logger::getLogger()->debug("Exiting Enhance::doOrder ownerid == 0");
      //currently not owned by anyone, just forget about it
      return false;
  }

  Game* game = Game::getGame();
  ResourceManager* resman = game->getResourceManager();
  const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
  const uint32_t resValue = planet->getResourceSurfaceValue(resType);
  const uint32_t enhanceValue = static_cast<uint32_t>(floor(points->getTime()/10));
  if (resValue >= points->getTime()) {
    if (planet->removeResource(resType, points->getTime())) {
      planet->addFactories(enhanceValue);
      Logger::getLogger()->debug("Enhance::doOrder Value(%d) Adding %d points to Factories next turn", resValue, enhanceValue);
      Logger::getLogger()->debug("Exiting Enhance::doOrder on success");
      return true;
    }
  } else {
    Message* message = new Message();
    message->setSubject("Not enough points for Enhance Order");
    message->setBody("There were not enough points on " + ob->getName() + " to fulfill your Enhance order.  Please check this order.");
    game->getPlayerManager()->getPlayer(planet->getOwner())->postToBoard(message);
  }
  return false;
}

Order* Enhance::clone() const{
  Enhance* nb = new Enhance();
  nb->type = type;
  return nb;
}

}
