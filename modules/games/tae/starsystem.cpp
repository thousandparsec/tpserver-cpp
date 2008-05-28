/*  Star System Object
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>

#include "spaceobject.h"

#include "starsystem.h"

StarSystemType::StarSystemType() : SpaceObjectType(){
    setTypeName("Star System");
    setTypeDescription("A Star System Object");

    ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
    group->setName("Status");
    group->setDescription("Information on the current status of the star system");
    group->addParameter(obpT_Integer, "Inhabitable", "Is the system inhabitable"); 
    group->addParameter(obpT_Integer, "Destroyed", "Is the star destroyed");
    addParameterGroupDesc(group);
}

StarSystemType::~StarSystemType() {
}

void StarSystemType::setTypeName(const std::string& n){
  nametype = n;
}

void StarSystemType::setTypeDescription(const std::string& d){
  typedesc = d;
}

ObjectBehaviour* StarSystemType::createObjectBehaviour() const{
  return new StarSystem();
}

StarSystem::StarSystem() : SpaceObject() {
}

StarSystem::~StarSystem() {

}

bool StarSystem::isInhabitable() {
    return (bool)((IntegerObjectParam*)(obj->getParameter(2,1)))->getValue();
}

void StarSystem::setInhabitable(bool inhabit) {
    ((IntegerObjectParam*)(obj->getParameter(2,1)))->setValue((uint32_t)inhabit);
    obj->touchModTime();
}

bool StarSystem::isDestroyed() {
    return (bool)((IntegerObjectParam*)(obj->getParameter(2,2)))->getValue();
}

void StarSystem::setDestroyed(bool des) {
    ((IntegerObjectParam*)(obj->getParameter(2,2)))->setValue((uint32_t)des);
    obj->touchModTime();
}

void StarSystem::doOnceATurn() {
}

int StarSystem::getContainerType() {
   return 2;
}
