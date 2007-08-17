/*  static object
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

#include "map.h"
#include "containertypes.h"

#include "staticobject.h"

namespace RFTS_ {

using std::pair;

StaticObject::StaticObject() : ObjectData(), unitX(0), unitY(0) {
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

void StaticObject::setUnitPos(double x, double y) {
   unitX = x, unitY = y;
   setPosition(getUniverseCoord(x,y));
}

void StaticObject::setUnitPos(const std::pair<double,double>& unitPos) {
   unitX = unitPos.first;
   unitY = unitPos.second;
   setPosition(getUniverseCoord(unitPos));
}

pair<double,double> StaticObject::getUnitPos() const {
   return pair<double,double>(unitX, unitY);
}

Vector3d StaticObject::getPosition() const{
  return pos->getPosition();
}

void StaticObject::setPosition(const Vector3d & np){
  pos->setPosition(np);
  touchModTime();
}

uint64_t StaticObject::getSize() const{
  return size->getSize();
}

void StaticObject::setSize(uint64_t ns){
  size->setSize(ns);
  touchModTime();
}

void StaticObject::packExtraData(Frame * frame) {

}

void StaticObject::doOnceATurn(IGObject * obj) {

}

ObjectData* StaticObject::clone() const {
  StaticObject* eo = new StaticObject();
  eo->nametype = nametype;
  eo->typedesc = typedesc;
  return eo;
}

int StaticObject::getContainerType(){
  return ContainerTypes_::StaticObject;
}

void StaticObject::setTypeName(const std::string& n){
  nametype = n;
}

void StaticObject::setTypeDescription(const std::string& d){
  typedesc = d;
}

}
