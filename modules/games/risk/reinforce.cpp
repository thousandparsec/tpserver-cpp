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
 
#include <tpserver/orderparameters.h>
#include <tpserver/message.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/designstore.h>
#include <tpserver/objectmanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/logging.h>
#include <tpserver/message.h>

#include <boost/format.hpp>
#include "reinforce.h"
#include "planet.h"
#include "risk.h"

namespace RiskRuleset {

using std::string;
using boost::format;

Reinforce::Reinforce() {
   name = "Reinforce";
   description = "Reinforce a planet";

   numberUnits = (TimeParameter*) addOrderParameter( new TimeParameter("Units","The number of units to reinforce with.") );

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
   uint32_t requestedUnits = numberUnits->getTime(); //since order is a "time" order
   uint32_t availibleUnits = risk->getPlayerReinforcements(planet->getOwner());
   --turns;
   
   //if user has asked for too many units then only give them as much as they can afford
   if ( requestedUnits > availibleUnits ) { 
      requestedUnits = availibleUnits;    
   }  

   //Give resources to planet
   planet->addResource("Army",requestedUnits);

   //Decrement players reinforcements total.
   risk->setPlayerReinforcements(planet->getOwner(), availibleUnits - requestedUnits);

   //Construct Reinforcement message
   Message::Ptr message( new Message() );
   message->setSubject("Reinforce order completed successfully.");
   format body("%1% received %2% additional units. %3% reinforcements remaining.");
   body % planet->getName(); body % requestedUnits; body % (availibleUnits - requestedUnits);
   message->setBody(body.str());

   //Post message to players board
   Game::getGame()->getPlayerManager()->getPlayer(planet->getOwner())->postToBoard(message);
   
   return result;
}

} //namespace RiskRuleset
