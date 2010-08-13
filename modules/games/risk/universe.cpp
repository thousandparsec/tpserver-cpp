/*  universe
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

#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/object.h>

#include "containertypes.h"

#include "universe.h"

namespace RiskRuleset {

UniverseType::UniverseType() : StaticObjectType( "Universe", "The Universe" ) {
  ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc( "Informational","Information about the universe");
   group->addParameter(obpT_Integer, "Year", "The current turn number");
}

ObjectBehaviour* UniverseType::createObjectBehaviour() const{
   return new Universe();
}

Universe::Universe() : StaticObject() {
}

void Universe::setTurn(int turn) {
   ((IntegerObjectParam*)(obj->getParameter(2, 1)))->setValue(turn);
}

int Universe::getTurn() const {
   return ((IntegerObjectParam*)(obj->getParameter(2, 1)))->getValue();
}

int Universe::getContainerType() {
   return ContainerTypes_::Universe;
}

void Universe::packExtraData(OutputFrame::Ptr frame) {
   frame->packInt(((IntegerObjectParam*)(obj->getParameter(2, 1)))->getValue());
}

void Universe::doOnceATurn() {
   ((IntegerObjectParam*)(obj->getParameter(2, 1)))->setValue(((IntegerObjectParam*)(obj->getParameter(2, 1)))->getValue() + 1);
   obj->touchModTime();
}

void Universe::setupObject(){
   setSize(0xffffffffffffffffULL);
}

}
