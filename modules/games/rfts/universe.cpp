/*  universe
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

#include <tpserver/frame.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroup.h>
#include <tpserver/sizeobjectparam.h>

#include "containertypes.h"

#include "universe.h"

namespace RFTS_ {

Universe::Universe() : StaticObject() {

   turn = new IntegerObjectParam();
   turn->setName("Turn");
   turn->setDescription("The current turn number");
   ObjectParameterGroup *group = new ObjectParameterGroup();
   group->setGroupId(2);
   group->setName("Informational");
   group->setDescription("Information about the universe");
   group->addParameter(turn);
   paramgroups.push_back(group);

   setSize(0xffffffffffffffffULL);

   nametype = "Universe";
   typedesc = "The Universe";
}

void Universe::setTurn(int turn) {
   this->turn->setValue(turn);
}

int Universe::getTurn() const {
   return turn->getValue();
}

ObjectData* Universe::clone() const {
   return new Universe();
}

int Universe::getContainerType() {
   return ContainerTypes_::Universe;
}

void Universe::packExtraData(Frame *frame) {
   frame->packInt(turn->getValue());
}

void Universe::doOnceATurn(IGObject *obj) {
   turn->setValue(turn->getValue() + 1);
   touchModTime();
}

}
