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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <tpserver/frame.h>
#include <tpserver/position3dobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/sizeobjectparam.h>
#include <tpserver/object.h>

#include "containertypes.h"

#include "wormhole.h"

namespace RiskRuleset {

WormholeType::WormholeType() : StaticObjectType() {
   ObjectParameterGroupDesc *group = new ObjectParameterGroupDesc();
   group->setName("Informational");
   group->setDescription("Information about the wormhole");
   group->addParameter(obpT_Position_3D, "Exit", "Where the wormhole exits.");
   addParameterGroupDesc(group);

   nametype = "Wormhole";
   typedesc = "Holes in the fabric of space allowing instant travel";
}

ObjectBehaviour* WormholeType::createObjectBehaviour() const{
   return new Wormhole();
}

Wormhole::Wormhole() : StaticObject() {
}

void Wormhole::setExit(const Vector3d &npos) {
   ((Position3dObjectParam*)(obj->getParameter(2, 1)))->setPosition(npos);
}

Vector3d Wormhole::getExit() {
   return ((Position3dObjectParam*)(obj->getParameter(2, 1)))->getPosition();
}

int Wormhole::getContainerType() {
   return ContainerTypes_::StaticObject;
}

void Wormhole::packExtraData(Frame *frame) {
   Vector3d end = getExit();
   frame->packInt64(end.getX());
   frame->packInt64(end.getY());
   frame->packInt64(end.getZ());
}

void Wormhole::doOnceATurn() {}

void Wormhole::setupObject(){
   setSize(0x1L);
}

}
