/*  renamefleet
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

#include <tpserver/object.h>
#include <tpserver/objectdata.h>

#include <tpserver/stringparameter.h>

#include "fleet.h"

#include "renamefleet.h"

namespace RFTS_ {

RenameFleet::RenameFleet() {
   name = "Rename Fleet";
   description = "Rename this fleet";

   newName = new StringParameter();
   newName->setName("new name");
   newName->setDescription("The new name of the fleet");
   addOrderParameter(newName);

   turns = 1;
}

RenameFleet::~RenameFleet() {

}

Order* RenameFleet::clone() const {
   RenameFleet *rf = new RenameFleet();
   rf->type = this->type;
   return rf;
}

bool RenameFleet::doOrder(IGObject *obj) {

   obj->setName(newName->getString());
   
   return true;
}


}
