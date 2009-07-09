/*  Fleet object
 *
 *  Copyright (C) 2004-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/mediaobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>

#include "spaceobject.h"

SpaceObjectType::SpaceObjectType():ObjectType(){
  ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
  group->setName("Positional");
  group->setDescription("Positional information");
  group->addParameter(obpT_Position_3D, "Position", "The position of the object");
  group->addParameter(obpT_Velocity, "Velocity", "The velocity of the object");
  group->addParameter(obpT_Size, "Size", "The size of the object");
  addParameterGroupDesc(group);
  group = new ObjectParameterGroupDesc();
  group->setName("Media");
  group->setDescription("Media for this object");
  group->addParameter(obpT_Media, "Icon", "Icon for this object");
  group->addParameter(obpT_Media, "Media", "The main media for the object");
  addParameterGroupDesc(group);
  
}

SpaceObjectType::~SpaceObjectType(){
}


const uint32_t SpaceObject::POSGRPID = 1;
const uint32_t SpaceObject::POSPARAMID = 1;
const uint32_t SpaceObject::VELPARAMID = 2;
const uint32_t SpaceObject::SIZEPARAMID = 3;
const uint32_t SpaceObject::MEDIAGRPID = 2;
const uint32_t SpaceObject::ICONPARAMID = 1;
const uint32_t SpaceObject::MEDIAPARAMID = 2;

SpaceObject::SpaceObject() : ObjectBehaviour(){
}

SpaceObject::~SpaceObject(){
}

Vector3d SpaceObject::getPosition() const{
  return ((Position3dObjectParam*)(obj->getParameter(POSGRPID,POSPARAMID)))->getPosition();
}

Vector3d SpaceObject::getVelocity() const{
  return ((Velocity3dObjectParam*)(obj->getParameter(POSGRPID,VELPARAMID)))->getVelocity();
}

uint64_t SpaceObject::getSize() const{
  return ((SizeObjectParam*)(obj->getParameter(POSGRPID,SIZEPARAMID)))->getSize();
}

void SpaceObject::setPosition(const Vector3d & np){
  ((Position3dObjectParam*)(obj->getParameter(POSGRPID,POSPARAMID)))->setPosition(np);
  obj->touchModTime();
}

void SpaceObject::setVelocity(const Vector3d & nv){
  ((Velocity3dObjectParam*)(obj->getParameter(POSGRPID,VELPARAMID)))->setVelocity(nv);
  obj->touchModTime();
}

void SpaceObject::setSize(uint64_t ns){
  ((SizeObjectParam*)(obj->getParameter(POSGRPID,SIZEPARAMID)))->setSize(ns);
  obj->touchModTime();
}

void SpaceObject::setIcon(const std::string& aMediaStr){
    ((MediaObjectParam*)(obj->getParameter(MEDIAGRPID, ICONPARAMID)))->setMediaUrl(aMediaStr);
    obj->touchModTime();
}

void SpaceObject::setMedia(const std::string& aMediaStr){
    ((MediaObjectParam*)(obj->getParameter(MEDIAGRPID, MEDIAPARAMID)))->setMediaUrl(aMediaStr);
    obj->touchModTime();
}

