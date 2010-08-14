/*  static object
 *
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

#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/mediaobjectparam.h>
#include <tpserver/object.h>

#include "map.h"
#include "containertypes.h"

#include "staticobject.h"

namespace RiskRuleset {

using std::pair;

StaticObjectType::StaticObjectType( const std::string& nname, const std::string& ndesc ) : ObjectType(nname,ndesc){
  ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc( "Positional", "Describes the position");
   group->addParameter(obpT_Position_3D, "Position", "The position of the object");
   group->addParameter(obpT_Size, "Size", "The diameter of the object");
   group->addParameter(obpT_Media, "Icon", "Icon for this object");
   group->addParameter(obpT_Media, "Media", "The media for this object"); 

}

ObjectBehaviour* StaticObjectType::createObjectBehaviour() const{
   return new StaticObject();
}

StaticObject::StaticObject() : ObjectBehaviour(), unitX(0), unitY(0) {
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
   return ((Position3dObjectParam*)(obj->getParameter(1, 1)))->getPosition();
}

void StaticObject::setPosition(const Vector3d & np){
   ((Position3dObjectParam*)(obj->getParameter(1, 1)))->setPosition(np);
   obj->touchModTime();
}

uint64_t StaticObject::getSize() const{
   return ((SizeObjectParam*)(obj->getParameter(1, 2)))->getSize();
}

void StaticObject::setSize(uint64_t ns){
   ((SizeObjectParam*)(obj->getParameter(1, 2)))->setSize(ns);
   obj->touchModTime();
}

void StaticObject::packExtraData(OutputFrame::Ptr frame) {

}

void StaticObject::doOnceATurn() {

}

int StaticObject::getContainerType(){
   return ContainerTypes_::StaticObject;
}

void StaticObject::setIcon(const std::string& media){
    ((MediaObjectParam*)(obj->getParameter(1,3)))->setMediaUrl(media);
}

void StaticObject::setMedia(const std::string& media){
    ((MediaObjectParam*)(obj->getParameter(1,4)))->setMediaUrl(media);
}

}
