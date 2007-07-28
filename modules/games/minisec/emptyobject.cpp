/*  Empty Object class
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/objectparametergroup.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>

#include "emptyobject.h"

EmptyObject::EmptyObject() : ObjectData(){
  pos = new Position3dObjectParam();
  pos->setName("Position");
  pos->setDescription("The position of the object");
  ObjectParameterGroup *group = new ObjectParameterGroup();
  group->setGroupId(1);
  group->setName("Positional");
  group->setDescription("Describes the position");
  group->addParameter(pos);
  size = new SizeObjectParam();
  size->setName("Size");
  size->setDescription("The diameter of the object");
  group->addParameter(size);
  paramgroups.push_back(group);
}

Vector3d EmptyObject::getPosition() const{
  return pos->getPosition();
}

uint64_t EmptyObject::getSize() const{
  return size->getSize();
}

void EmptyObject::setPosition(const Vector3d & np){
  pos->setPosition(np);
  touchModTime();
}

void EmptyObject::setSize(uint64_t ns){
  size->setSize(ns);
  touchModTime();
}

void EmptyObject::packExtraData(Frame * frame)
{

}

void EmptyObject::doOnceATurn(IGObject * obj)
{

}

ObjectData* EmptyObject::clone() const{
  EmptyObject* eo = new EmptyObject();
  eo->nametype = nametype;
  eo->typedesc = typedesc;
  return eo;
}

int EmptyObject::getContainerType(){
  return 1;
}

void EmptyObject::setTypeName(const std::string& n){
  nametype = n;
}

void EmptyObject::setTypeDescription(const std::string& d){
  typedesc = d;
}
