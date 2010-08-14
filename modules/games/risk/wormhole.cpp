/*  wormhole
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
 *  You should have receivuniverse->setIcon("common/object-icons/system");
   universe->setMedia("common-2d/foreign/freeorion/nebula-small/nebula3");ed a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <tpserver/position3dobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/mediaobjectparam.h>
#include <tpserver/object.h>

#include "containertypes.h"

#include "wormhole.h"

namespace RiskRuleset {

WormholeType::WormholeType() : ObjectType( "Wormhole", "Holes in the fabric of space allowing instant travel" ) {
    ObjectParameterGroupDesc::Ptr group = createParameterGroupDesc( "Positional", "Describes the position");
   group->addParameter(obpT_Position_3D, "EndA", "One end of the wormhold");
   group->addParameter(obpT_Position_3D, "EndB", "The other end of the wormhole.");
   group = createParameterGroupDesc("Media", "Media information");
   group->addParameter(obpT_Media, "Icon", "The icon for this object");

}

ObjectBehaviour* WormholeType::createObjectBehaviour() const{
   return new Wormhole();
}

Wormhole::Wormhole() : ObjectBehaviour() {
}

void Wormhole::setEndA(const Vector3d &npos) {
   ((Position3dObjectParam*)(obj->getParameter(1, 1)))->setPosition(npos);
}

void Wormhole::setEndB(const Vector3d &npos) {
   ((Position3dObjectParam*)(obj->getParameter(1, 2)))->setPosition(npos);
}

Vector3d Wormhole::getEndA() {
   return ((Position3dObjectParam*)(obj->getParameter(1, 1)))->getPosition();
}

Vector3d Wormhole::getEndB() {
   return ((Position3dObjectParam*)(obj->getParameter(1, 2)))->getPosition();
}

int Wormhole::getContainerType() {
   return ContainerTypes_::StaticObject;
}

void Wormhole::packExtraData(OutputFrame::Ptr frame) {
   Vector3d end = getEndB();
   frame->packInt64(end.getX());
   frame->packInt64(end.getY());
   frame->packInt64(end.getZ());
}

void Wormhole::doOnceATurn() {}

void Wormhole::setupObject(){
    ((MediaObjectParam*)(obj->getParameter(2,1)))->setMediaUrl("common/object-icons/link");
}

}
