/*  Universe object
 *
 *  Copyright (C) 2003-2004, 2007  Lee Begg and the Thousand Parsec Project
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

#include "universe.h"

Universe::Universe() : ObjectData(){
  year = new IntegerObjectParam();
  year->setName("Year");
  year->setDescription("The Age of the universe");
  pos = new Position3dObjectParam();
  pos->setName("Position");
  pos->setDescription("The position of the universe");
  ObjectParameterGroup *group = new ObjectParameterGroup();
  group->setGroupId(1);
  group->setName("Positional");
  group->setDescription("Describes the position");
  group->addParameter(pos);
  size = new SizeObjectParam();
  size->setName("Size");
  size->setDescription("The diameter of the universe");
  size->setSize(0xffffffffffffffffULL);
  group->addParameter(size);
  paramgroups.push_back(group);
  group = new ObjectParameterGroup();
  group->setGroupId(2);
  group->setName("Informational");
  group->setDescription("Information about the universe");
  group->addParameter(year);
  paramgroups.push_back(group);
  nametype = "Universe";
  typedesc = "The Universe";
  touchModTime();
}

Universe::~Universe(){
}

Vector3d Universe::getPosition() const{
  return pos->getPosition();
}

uint64_t Universe::getSize() const{
  return size->getSize();
}

void Universe::setPosition(const Vector3d & np){
  pos->setPosition(np);
  touchModTime();
}

void Universe::setSize(uint64_t ns){
  size->setSize(ns);
  touchModTime();
}

void Universe::packExtraData(Frame * frame){
  frame->packInt(year->getValue());
}

void Universe::doOnceATurn(IGObject * obj){
  year->setValue(year->getValue() + 1);
  touchModTime();
}

int Universe::getContainerType(){
  return 1;
}

ObjectData* Universe::clone() const{
  return new Universe();
}

void Universe::setYear(int nyear){
  year->setValue(nyear);
}

int Universe::getYear(){
  return year->getValue();
}
