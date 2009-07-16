/*  SendPoints object for Send Points orders
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
#include <tpserver/listparameter.h>
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

  targetPlanet = new ListParameter();
  targetPlanet->setName("Planet");
  targetPlanet->setDescription("The Planet to send points to.");
  targetPlanet->setListOptionsCallback(ListOptionCallback(this, &SendPoints::generateListOptions));
  addOrderParameter(targetPlanet);

  maxPoints = 100;

  turns = 1;  //the penalty is enough to warrant one turn use
}

SendPoints::~SendPoints(){
}

double SendPoints::getPercentage(IGObject *ob) const{
  Logger::getLogger()->debug("Enter SendPoints::getPercentage");
//  Planet* planet = (dynamic_cast<Planet*>(ob->getObjectBehaviour()));
//  uint64_t distance = coords->getPosition().getDistance(planet->getPosition());
//  if(distance == 0) 
//    return 1;
  return 0.5; //temporary return value
}


bool SendPoints::doOrder(IGObject *ob)
{
  Logger::getLogger()->debug("Entering SendPoints::doOrder");
  Game* game = Game::getGame();
  ObjectManager* obman = game->getObjectManager();
  Planet* source = dynamic_cast<Planet*>(ob->getObjectBehaviour());

  std::map<uint32_t,uint32_t> list = targetPlanet->getList();
  Logger::getLogger()->debug("SendPoints::doOrder List Size: %d", list.size());
  for(std::map<uint32_t,uint32_t>::iterator i = list.begin(); i != list.end(); ++i) {
    uint32_t destID = i->first;
    uint32_t numRes = i->second;

    IGObject* destObj = obman->getObject(destID);
    Planet* destPlanet = dynamic_cast<Planet*>(destObj->getObjectBehaviour());
    if (destPlanet->getOwner() != 0) {
      Logger::getLogger()->debug("Found Planet(%s), Sending %d Points", destObj->getName().c_str(), i->second);
      ResourceManager* resman = game->getResourceManager();
      const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
      if (source->removeResource(resType, numRes) == true) {
        destPlanet->addFactoriesNextTurn(numRes);
        return true;
      }

    }
  }
  return false;
}

std::map<uint32_t, std::pair<std::string, uint32_t> > SendPoints::generateListOptions() {
   std::map<uint32_t, std::pair<std::string, uint32_t> > options;
   Game* game = Game::getGame();
   ObjectManager* obman = game->getObjectManager();
   ObjectTypeManager* obtm = game->getObjectTypeManager();
   ResourceManager* resman = game->getResourceManager();

   IGObject* selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   Planet* planet = dynamic_cast<Planet*>(selectedObj->getObjectBehaviour());
   assert(planet);
   game->getObjectManager()->doneWithObject(selectedObj->getID());

   std::set<uint32_t> ids = obman->getAllIds();

   for(std::set<uint32_t>::iterator i = ids.begin(); i != ids.end(); i++) {
      IGObject* curObject = obman->getObject(*i);
      if (curObject->getType() == obtm->getObjectTypeByName("Planet")) {
         Planet* curPlanet = dynamic_cast<Planet*>(curObject->getObjectBehaviour());
         if (curPlanet->getOwner() != 0) {
            const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
            options[curObject->getID()] = std::pair<std::string, uint32_t>(curObject->getName(), planet->getResourceSurfaceValue(resType));
         }
      }
   }   

   return options;
}



Order* SendPoints::clone() const{
  SendPoints* nb = new SendPoints();
  nb->type = type;
  return nb;
}

