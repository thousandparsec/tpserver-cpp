/*  Reinforce
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
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
 
#include <tpserver/objectorderparameter.h>
#include <tpserver/message.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/designstore.h>
#include <tpserver/objectmanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/logging.h>
#include <tpserver/timeparameter.h>
#include <tpserver/message.h>

#include "reinforce.h"
#include "planet.h"
#include "risk.h"

namespace RiskRuleset {

using std::string;

Reinforce::Reinforce() {
   name = "Reinforce";
   description = "Reinforce a planet";

   numberUnits = new TimeParameter();
   numberUnits->setName("Units");
   numberUnits->setDescription("The number of units to reinforce with.");
   addOrderParameter(numberUnits);

   turns = 1;
}

Reinforce::~Reinforce() {

}

Order* Reinforce::clone() const {
   Reinforce *c = new Reinforce();
   c->type = this->type;
   return c;
}

bool Reinforce::doOrder(IGObject *obj) {
   Logger::getLogger()->debug("Starting a Reinforce::doOrder");
   bool result = true;
   
   Planet* planet = dynamic_cast<Planet*>(obj->getObjectBehaviour());
   assert(planet);
   
   Risk* risk = dynamic_cast<Risk*>(Game::getGame()->getRuleset());
   uint32_t requestedUnits = numberUnits->getTime();
   uint32_t availibleUnits = risk->getPlayerReinforcements(planet->getOwner());
   --turns;
   
   //if user has asked for too many units then only give them as much as they can afford
   if ( requestedUnits > availibleUnits ) { 
      requestedUnits = availibleUnits;    
   }  

   planet->addResource("Army",requestedUnits);

   //Decrement players reinforcements total.
   risk->setPlayerReinforcements(planet->getOwner(), availibleUnits - requestedUnits);

   //TODO: Inform player how many reinforcements were added, their new reinforcement total.
   //ASK: about getting a string out of a uint32_t
   return true;
}

} //namespace RiskRuleset
