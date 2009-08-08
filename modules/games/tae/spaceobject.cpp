/*  Space object
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <math.h>

#include <tpserver/object.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/velocity3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>

#include "spaceobject.h"

SpaceObjectType::SpaceObjectType( const std::string& nname, const std::string& ndesc ):ObjectType(nname,ndesc){
  ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
  group->setName("Positional");
  group->setDescription("Positional information");
  group->addParameter(obpT_Position_3D, "Position", "The position of the object");
  group->addParameter(obpT_Velocity, "Velocity", "The velocity of the object");
  group->addParameter(obpT_Size, "Size", "The size of the object");
  addParameterGroupDesc(group);
  
}

SpaceObjectType::~SpaceObjectType(){
}

SpaceObject::SpaceObject() : ObjectBehaviour(){
}

SpaceObject::~SpaceObject(){
}

Vector3d SpaceObject::getPosition() const{
  return ((Position3dObjectParam*)(obj->getParameter(1,1)))->getPosition();
}

Vector3d SpaceObject::getVelocity() const{
  return ((Velocity3dObjectParam*)(obj->getParameter(1,2)))->getVelocity();
}

uint64_t SpaceObject::getSize() const{
  return ((SizeObjectParam*)(obj->getParameter(1,3)))->getSize();
}

void SpaceObject::setPosition(const Vector3d & np){
  ((Position3dObjectParam*)(obj->getParameter(1,1)))->setPosition(np);
  obj->touchModTime();
}

void SpaceObject::setVelocity(const Vector3d & nv){
  ((Velocity3dObjectParam*)(obj->getParameter(1,2)))->setVelocity(nv);
  obj->touchModTime();
}

void SpaceObject::setSize(uint64_t ns){
  ((SizeObjectParam*)(obj->getParameter(1,3)))->setSize(ns);
  obj->touchModTime();
}


