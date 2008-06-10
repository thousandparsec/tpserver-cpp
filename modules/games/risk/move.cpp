/*  move class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
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

#include <tpserver/frame.h>
#include <tpserver/objectorderparameter.h>
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
#include <tpserver/listparameter.h>

#include "move.h"
#include "planet.h"

//TODO: Make Move order display correctly in client
namespace RiskRuleset {

using std::map;
using std::pair;
using std::string;
using std::set;

Move::Move() : Order() {
   name = "Move";
   description = "Move any number of units to an adjacent planet";

   targetPlanet = new ListParameter();
   targetPlanet->setName("Planet");
   targetPlanet->setDescription("The Planet to move to.");
   targetPlanet->setListOptionsCallback(ListOptionCallback(this,
      &Move::generateListOptions));
   addOrderParameter(targetPlanet);

   turns = 1;
}

Move::~Move() {

}

//This function is commented to aid new ruleset developers in understanding
//how to construct a generateListOptions function
map<uint32_t, pair<string, uint32_t> >Move::generateListOptions() {
   //This map will be filled with options to be displayed
   //The pair is made up of a title and an integer max
   map<uint32_t, pair<string,uint32_t> > options;
   
   Game* game = Game::getGame();
   
   //Get the current object in question. This is done so we have access
   //to the object in which the list is being generated for, letting us
   //populate the list in a context specific matter 
   IGObject* selectedObj = game->getObjectManager()->getObject(
      game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
   //In the Risk ruleset the Move order is only used on Planets, and here
   //I capture a pointer to the Planet my IGObject is.
   Planet* planet = dynamic_cast<Planet*>(selectedObj->getObjectBehaviour());
   
   //Here we ensure that the dynamic_cast went ok
   assert(planet);
   
   //And now we tell the object manager we are done with our selectedObj'ect
   game->getObjectManager()->doneWithObject(selectedObj->getID());

   /* I'm now grabbing the set of planets adjacent to the current planet.
   This operation is fairly specific to risk, as risk restricts movement
   to adjacent territories, or in my case, planets. */
   set<Planet*> adjacent = planet->getAdjacent();

   /* I now grab the number of "Army" resources on the planet, as I intend to
   use it as a maximum for each pair I add to the map.
      I subtract 1 here because in Risk a player must keep at least one
      unit on each territory to maintain claim over it */
   uint32_t availibleUnits = planet->getResource("Army").first - 1;

   /* This for loop will iterate over every adjacent planet. 
   This is where the majority of the work occurs, and we populate our list.
   You see here we select an item of the map, in my case (*i)->getID(), and
   for that item we create a pair.
      If its a little hard to read, here is what I am doing:
            options[#] = pair<string,uint32_t>( "title", max# );

   For my pair I set the title as the adjacent planet to move to, and set the
   max to availible units. */
   for(set<Planet*>::iterator i = adjacent.begin(); i != adjacent.end(); i++) {
      options[(*i)->getID()] = pair<string,uint32_t>(
         (*i)->getName(), availibleUnits );
   }   

   return options;
}

Order* Move::clone() const {
   Move* o = new Move();
   o->type = type;
   return o;
}

bool Move::doOrder(IGObject* obj) {
   --turns;
   
   Game* game = Game::getGame();
   ObjectManager* om = game->getObjectManager();
   Planet* planet = dynamic_cast<Planet*>(obj->getObjectBehaviour());
   assert(planet);
   
   //Get the list of planetIDs and the # of units to move
   map<uint32_t,uint32_t> list = targetPlanet->getList();
   
   //Iterate over all planetIDs and # of units to move to them
   for(map<uint32_t,uint32_t>::iterator i = list.begin(); i != list.end(); ++i) {
      uint32_t planetID = i->first;
      uint32_t numUnits = i->second;
      Planet* origin = dynamic_cast<Planet*>(
         obj->getObjectBehaviour());
      assert(origin);
      Planet* target = dynamic_cast<Planet*>(
         om->getObject(planetID)->getObjectBehaviour());
      assert(target);
      
      if (origin->getOwner() == target->getOwner()) {
         //Friendly Move
         origin->removeResource("Army",numUnits);
         target->addResource("Army",numUnits);
      }
      //origin and target owners are not the same, target is owned
      else if (target->getOwner() != 0){ 
         //Attack Move
         
      }
      //origin and target owners are not the same, target is unowned
      else {
         //Colonize Move
         origin->removeResource("Army",numUnits);
         target->setOwner(origin->getOwner());
         target->addResource("Army",numUnits);
      }
         //Check to see if target planet has an order for attacking base planet
         //if so execute "balanced" roll (i.e. 3-3)
            //remove order on target planet to attack current planet
         //else execute "attacker-favored" roll (i.e. 3-2)

         //apply results of battle to base and target planets

         //if target planet is conquerred (no more armies on surface)
            //change owner of target planet to current planet owner
            //remove all orders on target planet
   }

   //TODO: send message about results of move order
   
   return true;   //moves are always completed, no matter what happens
}

} //end namespace RiskRuleset
