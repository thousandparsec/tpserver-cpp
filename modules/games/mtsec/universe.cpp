/*  Universe object
 *
 *  Copyright (C) 2003-2004, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/integerobjectparam.h>

#include "universe.h"

namespace MTSecRuleset {

UniverseType::UniverseType() : SpaceObjectType("Universe", "The Universe"){
  ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc( "Informational", "Information about the universe");
  group->addParameter(obpT_Integer, "Year", "The Age of the universe");
}

UniverseType::~UniverseType(){
}

ObjectBehaviour* UniverseType::createObjectBehaviour() const{
  return new Universe();
}

Universe::Universe(){
}

Universe::~Universe(){
}


void Universe::packExtraData(OutputFrame * frame){
  frame->packInt(dynamic_cast<IntegerObjectParam*>(obj->getParameter(2,1))->getValue());
}

void Universe::doOnceATurn(){
  dynamic_cast<IntegerObjectParam*>(obj->getParameter(2,1))->setValue(dynamic_cast<IntegerObjectParam*>(obj->getParameter(2,1))->getValue() + 1);
  obj->touchModTime();
}

int Universe::getContainerType(){
  return 1;
}


void Universe::setYear(int nyear){
  dynamic_cast<IntegerObjectParam*>(obj->getParameter(2,1))->setValue(nyear);
}

int Universe::getYear(){
  return dynamic_cast<IntegerObjectParam*>(obj->getParameter(2,1))->getValue();
}

}
