/*  Universe Object
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

#include "universe.h"

#include <tpserver/object.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/velocity3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/integerobjectparam.h>


UniverseType::UniverseType() : ObjectType( "Universe", "The Universe") 
{
  ObjectParameterGroupDesc::Ptr group;

  group = createParameterGroupDesc("Positional","Positional information");
  group->addParameter(obpT_Position_3D, "Position", "The position of the object");
  group->addParameter(obpT_Velocity, "Velocity", "The velocity of the object");
  group->addParameter(obpT_Size, "Size", "The size of the object");

  group = createParameterGroupDesc( "Informational", "Information about the universe");
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

void Universe::packExtraData(OutputFrame *frame) {
    frame->packInt(((IntegerObjectParam*)(obj->getParameter(2,1)))->getValue());
}

void Universe::doOnceATurn(){
  ((IntegerObjectParam*)(obj->getParameter(2,1)))->setValue(((IntegerObjectParam*)(obj->getParameter(2,1)))->getValue() + 1);
  obj->touchModTime();
}

int Universe::getContainerType(){
    return 1;
}

Vector3d Universe::getPosition() const{
    return ((Position3dObjectParam*)(obj->getParameter(1,1)))->getPosition();
}

Vector3d Universe::getVelocity() const{
    return ((Velocity3dObjectParam*)(obj->getParameter(1,2)))->getVelocity();
}

uint64_t Universe::getSize() const{
    return ((SizeObjectParam*)(obj->getParameter(1,3)))->getSize();
}

void Universe::setPosition(const Vector3d & np){
    ((Position3dObjectParam*)(obj->getParameter(1,1)))->setPosition(np);
    obj->touchModTime();
}
void Universe::setVelocity(const Vector3d & nv){
    ((Velocity3dObjectParam*)(obj->getParameter(1,2)))->setVelocity(nv);
    obj->touchModTime();
}
void Universe::setSize(uint64_t ns){
    ((SizeObjectParam*)(obj->getParameter(1,3)))->setSize(ns);
    obj->touchModTime();
}
void Universe::setYear(int nyear){
    ((IntegerObjectParam*)(obj->getParameter(2,1)))->setValue(nyear);
    obj->touchModTime();
}
int Universe::getYear(){
    return ((IntegerObjectParam*)(obj->getParameter(2,1)))->getValue();
}
