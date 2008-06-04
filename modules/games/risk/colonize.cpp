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
#include <tpserver/message.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/designstore.h>
#include <tpserver/objectmanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>

#include "colonize.h"

namespace RiskRuleset {

using std::string;

Colonize::Colonize() {
   name = "Colonize";
   description = "Colonize a planet";

   //ASK: how to ensure colonize order is availible on unowned planets
   //CHECK: on validity of these parameters
   units = new ObjectOrderParameter();
   units->setName("Units");
   units->setDescription("The number of units to colonize with.");
   addOrderParameter(units);

}

Colonize::~Colonize() {

}

Order* Colonize::clone() const {
   Colonize *c = new Colonize();
   c->type = this->type;
   return c;
}

bool Colonize::doOrder(IGObject *obj) {
   bool result = true;
   //TODO: Implement order
 
   //--turns; 
   //Check object for other Colonize orders, pick the largest VERIFIED bid (must check player has reinforcements availible)
   //Change object owner to owner of largest "bid"
   //Add # of armies in bid to planet
   //Clear all remaining colonize orders (there shouldn't be other order types on the planet though)
   //Inform colonize winner, as well as other bidders, of the results. Inform winner of new reinforcement total.
   return result;
}

}
