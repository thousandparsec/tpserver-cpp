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
  
  //TODO: Check validity of order parameters
  starSys = new ObjectOrderParameter();
  starSys->setName("Star System");
  starSys->setDescription("The star system to move to.");
  addOrderParameter(starSys);
  units = new ObjectOrderParameter();
  units->setName("Units");
  units->setDescription("The number of units to colonize with.");
  addOrderParameter(units);
  
  //need this?
  turns = 1;
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
  return result;
}
} //end namespace RiskRuleset
