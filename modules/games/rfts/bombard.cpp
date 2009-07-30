/*  bombard
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
#include <tpserver/objecttypemanager.h>
#include <tpserver/frame.h>
#include <tpserver/message.h>
#include <tpserver/logging.h>
#include <tpserver/prng.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>

#include <tpserver/orderparameters.h>

#include "planet.h"
#include "fleet.h"
#include "playerinfo.h"

#include "bombard.h"

namespace RFTS_ {

using std::string;

Bombard::Bombard() {
   name = "Bombard";
   description = "Bombard a planet, damaging it's resources";

   planet = (ObjectOrderParameter*)addOrderParameter( new ObjectOrderParameter( "Planet", "The planet to bombard") );

   turns = 1;
}

Bombard::~Bombard() {

}

Order* Bombard::clone() const {
   Bombard *b = new Bombard();
   b->type = this->type;
   return b;
}

void Bombard::inputFrame(Frame * f, uint32_t playerid) {
   Order::inputFrame(f, playerid);

   ObjectManager *om = Game::getGame()->getObjectManager();

   IGObject *planetObj = om->getObject(planet->getObjectId());
   Planet *planetData = dynamic_cast<Planet*>(planetObj->getObjectBehaviour());

   if(!planetData)
      planet->setObjectId(0);
   else if(planetData->getOwner() == playerid)
      planet->setObjectId(0);
}

bool Bombard::doOrder(IGObject *fleet) {

   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();
   ObjectTypeManager *odm = game->getObjectTypeManager();
   Random *rand = game->getRandom();

   IGObject *planetObj = om->getObject(planet->getObjectId());

   if(planetObj->getType() != odm->getObjectTypeByName("Planet"))
   {
      Logger::getLogger()->debug("Player tried to bombard something illogical");
      return true;
   }

   Fleet *fleetData = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());
   Planet *planetData = dynamic_cast<Planet*>(planetObj->getObjectBehaviour());
   char planetShipTech = PlayerInfo::getPlayerInfo(planetData->getOwner()).getShipTechLevel();
   double attack = fleetData->getAttack();
   
   planetData->removeResource("Industry", 
                  static_cast<uint32_t>(attack * INDUSTRY_DMG * (1+rand->getInRange(-2,2)/100.) ));

   planetData->removeResource("Social Environment", 
                  static_cast<uint32_t>(attack * SOCIAL_DMG * (1+rand->getInRange(-2,2)/100.) ));

   planetData->removeResource("Planetary Environment", 
                  static_cast<uint32_t>(attack * PLANETARY_DMG * (1+rand->getInRange(-2,2)/100.) ));

   planetData->removeResource(string("PDB") + planetShipTech,
                  static_cast<uint32_t>(attack * PDB_DMG / 10. * (1+rand->getInRange(-2,2)/100.) ));

   PlayerInfo::getPlayerInfo(fleetData->getOwner()).addVictoryPoints(VICTORY_POINTS);


   // PDBs counter-attack at: PDB level * 6 * numPDBs
   fleetData->takeDamage( (planetShipTech - '0') *
                         6 * planetData->getResource(string("PDB") + planetShipTech).first);

   Message::Ptr msg( new Message() );
   msg->setSubject("Bombard complete");
   string body = "Fleet \"" + fleet->getName() + "\" bombarded " + planetObj->getName();
   msg->setBody(PlayerInfo::appAllVictoryPoints(body));
   msg->addReference(rst_Action_Order, rsorav_Completion);
   msg->addReference(rst_Object, fleet->getID());
   msg->addReference(rst_Object, planetObj->getID());

   game->getPlayerManager()->getPlayer(fleetData->getOwner())->postToBoard(msg);
   game->getPlayerManager()->getPlayer(planetData->getOwner())->postToBoard(Message::Ptr( new Message(*msg)));

   return true;
}

}
