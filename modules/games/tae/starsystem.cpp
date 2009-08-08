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
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>

#include "spaceobject.h"
#include "planet.h"

#include "starsystem.h"

using std::set;

StarSystemType::StarSystemType() : SpaceObjectType("Star System","A Star System Object") {

    ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
    group->setName("Status");
    group->setDescription("Information on the current status of the star system");
    group->addParameter(obpT_Integer, "Inhabitable", "Is the system inhabitable"); 
    group->addParameter(obpT_Integer, "Destroyed", "Is the star destroyed");
    group->addParameter(obpT_Integer, "Region", "The region this star system belongs to");
    addParameterGroupDesc(group);
}

StarSystemType::~StarSystemType() {
}

ObjectBehaviour* StarSystemType::createObjectBehaviour() const{
  return new StarSystem();
}

StarSystem::StarSystem() : SpaceObject() {
}

StarSystem::~StarSystem() {

}

//System cannot be destroyed, occupied, or colonized in order to be able to be colonized.
//It also must be inhabitable for the colony type.
bool StarSystem::canBeColonized(bool mining) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();

    if(isDestroyed()) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System is destroyed");
        return false;
    }

    //Grab the children and see if the system is occupied
    set<uint32_t> children = obj->getContainedObjects();
    uint32_t pid;
    bool isOccupied = false;
    for(set<uint32_t>::iterator i = children.begin(); i != children.end(); i++) {
        if(obm->getObject(*i)->getType() == obtm->getObjectTypeByName("Planet")) {
            pid = *i;
        } else if (obm->getObject(*i)->getType() == obtm->getObjectTypeByName("Fleet")) {
            isOccupied = true;
        }
    }
    if(isOccupied) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System is occupied");
        return false;
    }

    //Check to see if the system has already been colonized
    Planet* planet = (Planet*)(obm->getObject(pid)->getObjectBehaviour());
    if(planet->getResource(4) > 0) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System has been colonized by merchants.");
        return false;
    } else if(planet->getResource(5) > 0) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System has been colonized by scientists.");
        return false;
    } else if(planet->getResource(6) > 0) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System has been colonized by settlers.");
        return false;
    } else if(planet->getResource(7) > 0) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System has been colonized by mining robots.");
        return false;
    }

    //Check to make sure the system is inhabitable by the fleet
    if(mining && planet->getResource(1) < 1) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System is inhabitable and cannot be colonized by mining robots");
        return false;
    } else if(!mining && planet->getResource(1) > 0) {
        Logger::getLogger()->debug("StarSystem->canBeColonized: System is Uninhabitable and can ONLY be colonized by mining robots");
        return false;
    }

    return true;
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

uint32_t StarSystem::getRegion() {
    return ((IntegerObjectParam*)(obj->getParameter(2,3)))->getValue();
}

void StarSystem::setRegion(uint32_t region) {
    ((IntegerObjectParam*)(obj->getParameter(2,3)))->setValue(region);
    obj->touchModTime();
}   

void StarSystem::doOnceATurn() {
}

int StarSystem::getContainerType() {
   return 2;
}
