/*  colonize
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
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/message.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/logging.h>
#include <tpserver/listparameter.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderqueue.h>
#include <tpserver/settings.h>

#include "colonize.h"
#include "planet.h"

namespace RiskRuleset {

using std::map;
using std::pair;
using std::string;
using std::set;

Colonize::Colonize() : Order()  {
   name = "Colonize";
   description = "Colonize a planet";

   //ASK: Work with Lee to get colonize order available on unowned planets
   //CHECK: on validity of these parameters
   targetPlanet = new ListParameter();
   targetPlanet->setName("Planet");
   targetPlanet->setDescription("The Planet to bid on.");
   targetPlanet->setListOptionsCallback(ListOptionCallback(this,
      &Colonize::generateListOptions));
   addOrderParameter(targetPlanet);

   turns = 1;
}

Colonize::~Colonize() {

}

map<uint32_t, pair<string, uint32_t> > Colonize::generateListOptions() {
   map<uint32_t, pair<string,uint32_t> > options;
   Game* game = Game::getGame();
   ObjectManager* om = game->getObjectManager();

   IGObject* selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   Planet* planet = dynamic_cast<Planet*>(selectedObj->getObjectBehaviour());
   assert(planet);
   om->doneWithObject(selectedObj->getID());

   set<uint32_t> allObjs = om->getAllIds();

   uint32_t availibleUnits = planet->getResource("Army").second;

   /* This for loop will iterate over every adjacent planet. 
   This is where the majority of the work occurs, and we populate our list.
   You see here we select an item of the map, in my case (*i)->getID(), and
   for that item we create a pair.
      If its a little hard to read, here is what I am doing:
            options[#] = pair<string,uint32_t>( "title", max# );

   For my pair I set the title as the adjacent planet to move to, and set the
   max to availible units. */
   for(set<uint32_t>::iterator i = allObjs.begin(); i != allObjs.end(); i++) {
      IGObject* currObj = om->getObject((*i));
      Planet* owned = dynamic_cast<Planet*>(currObj->getObjectBehaviour());
      if ( owned != NULL && owned->getOwner() == 0) {   
      options[owned->getID()] = pair<string,uint32_t>(
         owned->getName(), availibleUnits );
      }
   }   
   //CHECK: how to get more than a single digit display

   return options;
}

Order* Colonize::clone() const {
   Colonize *c = new Colonize();
   c->type = this->type;
   return c;
}

bool Colonize::doOrder(IGObject *obj) {
   bool result = true;
   --turns;
   //TODO: Implement Colonize order
 
   //Check object for other Colonize orders, pick the largest VERIFIED bid (must check player has reinforcements availible)
   //Change object owner to owner of largest "bid"
   //Add # of armies in bid to planet
   //Clear all remaining colonize orders (there shouldn't be other order types on the planet though)
   //Inform colonize winner, as well as other bidders, of the results. Inform winner of new reinforcement total.
   return result;
}

}
