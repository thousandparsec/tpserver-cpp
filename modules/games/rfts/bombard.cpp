/*  bombard
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

#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectdata.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/frame.h>
#include <tpserver/message.h>
#include <tpserver/logging.h>
#include <tpserver/prng.h>

#include <tpserver/objectorderparameter.h>

#include "planet.h"
#include "fleet.h"

#include "bombard.h"

namespace RFTS_ {

Bombard::Bombard() {
   name = "Bombard";
   description = "Bombard a planet, damaging it's resources";

   planet = new ObjectOrderParameter();
   planet->setName("Planet");
   planet->setDescription("The planet to bombard");
   addOrderParameter(planet);

   turns = 1;
}

Bombard::~Bombard() {

}

Order* Bombard::clone() const {
   Bombard *b = new Bombard();
   b->type = this->type;
   return b;
}

Result Bombard::inputFrame(Frame * f, unsigned int playerid) {
   Result r = Order::inputFrame(f, playerid);
   if(!r) return r;

   ObjectManager *om = Game::getGame()->getObjectManager();

   IGObject *planetObj = om->getObject(planet->getObjectId());
   Planet *planetData = dynamic_cast<Planet*>(planetObj->getObjectData());

   if(!planetData)
      planet->setObjectId(0);
   else if(planetData->getOwner() == playerid)
      planet->setObjectId(0);

   return Success();
}

bool Bombard::doOrder(IGObject *fleet) {

   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();
   ObjectDataManager *odm = game->getObjectDataManager();
   Random *rand = game->getRandom();

   IGObject *planetObj = om->getObject(planet->getObjectId());

   if(planetObj->getType() != odm->getObjectTypeByName("Planet"))
   {
      Logger::getLogger()->debug("Player tried to bombard something illogical");
      return true;
   }

   Fleet *fleetData = dynamic_cast<Fleet*>(fleet->getObjectData());
   Planet *planetData = dynamic_cast<Planet*>(planetObj->getObjectData());

   double attack = fleetData->getAttack();

   planetData->removeResource("Population",
                              static_cast<uint32_t>(attack * POPULATION_DMG * rand->getReal1()));

   planetData->removeResource("Industry", 
                              static_cast<uint32_t>(attack * INDUSTRY_DMG * rand->getReal1()));

   planetData->removeResource("Social Environment", 
                              static_cast<uint32_t>(attack * SOCIAL_DMG * rand->getReal1()));

   planetData->removeResource("Planetary Environment", 
                              static_cast<uint32_t>(attack * PLANETARY_DMG * rand->getReal1()));

   

   return true;
}

}
