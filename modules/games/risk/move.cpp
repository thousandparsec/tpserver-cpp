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

#include "move.h"

namespace RiskRuleset {

using std::string;

Move::Move() : Order() {
   name = "Move";
   description = "Move any number of units to a planet";

   planet = new ObjectOrderParameter();
   planet->setName("Planet");
   planet->setDescription("The Planet to move to.");
   addOrderParameter(planet);   //TODO: Check how to populate this parameter with only adjacent planets
   units = new ObjectOrderParameter();
   units->setName("Units");
   units->setDescription("The number of units to move (or attack with.)");
   addOrderParameter(units);

}

Move::~Move() {

}

Order* Move::clone() const {
   Move* o = new Move();
   o->type = type;
   return o;
}

bool Move::doOrder(IGObject* obj) {
   bool result = true;
   //TODO: Implement order
   
   //Double check if target planet is in list of adjacent planets?

   //If planet is friendly - simply transfer units to planet
   //Else
      //Check to see if target planet has an order for attacking base planet
      //if so execute "balanced" roll (i.e. 3-3)
         //remove order on target planet to attack current planet
      //else execute "attacker-favored" roll (i.e. 3-2)

      //apply results of battle to base and target planets
   
      //if target planet is conquerred (no more armies on surface)
         //change owner of target planet to current planet owner
         //remove all orders on target planet

   //Send message to players (and target planet owner if applicable)
   
   return result;
}

} //end namespace RiskRuleset
