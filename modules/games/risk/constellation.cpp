/*  Constellation
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

#include <tpserver/frame.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/object.h>

#include "containertypes.h"

#include "constellation.h"

namespace RiskRuleset {

ConstellationType::ConstellationType() : StaticObjectType("Constellation", "A Constellation" ) {
  ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc( "Informational", "Information about the constellation");
   group->addParameter(obpT_Integer, "Bonus", "Reinforcement bonus for owning entire onstellation");
}

ObjectBehaviour* ConstellationType::createObjectBehaviour() const{
   return new Constellation();
}

Constellation::Constellation() : StaticObject() {
}

void Constellation::setBonus(int bonus) {
   ((IntegerObjectParam*)(obj->getParameter(2, 1)))->setValue(bonus);
   obj->touchModTime();
}

int Constellation::getBonus() const {
   return ((IntegerObjectParam*)(obj->getParameter(2, 1)))->getValue();
}

int Constellation::getContainerType() {
   return ContainerTypes_::Constellation;
}

void Constellation::doOnceATurn() {

   obj->touchModTime();
}

void Constellation::setupObject(){

}

}
