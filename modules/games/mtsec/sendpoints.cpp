/*  SendPoints object for Send Points orders
 *
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
#include <tpserver/message.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/playermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/spacecoordparam.h>
#include <tpserver/timeparameter.h>
#include <tpserver/objectmanager.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include "planet.h"

#include "sendpoints.h"

#define MAX(x,y) (x<y) ? (y) : (x)
#define MIN(x,y) (x<y) ? (x) : (y)

SendPoints::SendPoints() : Order()
{
  name = "Send Points";
  description = "Send Production Points";

  coords = new SpaceCoordParam();
  coords->setName("pos");
  coords->setDescription("The position in space to move to");
  addOrderParameter(coords);

  points = new TimeParameter();
  points->setName("Production Points");
  points->setMax(100);
  points->setDescription("The number of production points you want to send.");
  addOrderParameter(points);

  maxPoints = 100;

  turns = 1;  //the penalty is enough to warrant one turn use
}

SendPoints::~SendPoints(){
}

double SendPoints::getPercentage(IGObject *ob) const{
  Logger::getLogger()->debug("Enter SendPoints::getPercentage");
  Planet* planet = (dynamic_cast<Planet*>(ob->getObjectBehaviour()));
  uint64_t distance = coords->getPosition().getDistance(planet->getPosition());
  if(distance == 0) 
    return 1;
  Logger::getLogger()->debug("Exit SendPoints::getPercentage Distance: %lld", distance);
  return (distance%9)/10; //temporary return value
}


bool SendPoints::doOrder(IGObject *ob)
{
  Logger::getLogger()->debug("Entering SendPoints::doOrder");
  Vector3d dest = coords->getPosition();
  Game* game = Game::getGame();
  ObjectManager* obman = game->getObjectManager();
  ObjectTypeManager* obtm = game->getObjectTypeManager();
  std::set<uint32_t> ids = obman->getObjectsByPos(dest, 1);
  for (std::set<uint32_t>::iterator itcurr=ids.begin(); itcurr != ids.end(); itcurr++) {
    IGObject* temp = obman->getObject(*itcurr);
    if(temp->getType() == obtm->getObjectTypeByName("Planet")) {
      Planet* dest = static_cast<Planet*>(temp->getObjectBehaviour());
      Logger::getLogger()->debug("SendPoints::doOrder Found Planet at coordinates");
      Planet* source = static_cast<Planet*>(ob->getObjectBehaviour());
      ResourceManager* resman = game->getResourceManager();
      const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
      if (source->removeResource(resType, points->getTime()) == true) {
        dest->addResource(resType, points->getTime());
        return true;
      }
    }
  }
  return false;
}


Order* SendPoints::clone() const{
  SendPoints* nb = new SendPoints();
  nb->type = type;
  return nb;
}

