/*  Fleet object
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

#include <tpserver/frame.h>
#include <tpserver/order.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/objectparametergroupdesc.h>

#include "ownedobject.h"

OwnedObjectType::OwnedObjectType(const std::string& nname, const std::string& ndesc):SpaceObjectType(nname,ndesc){
  ObjectParameterGroupDesc::Ptr group;
  group = createParameterGroupDesc( "Ownership", "The ownership of this object");
  group->addParameter(obpT_Reference, "Owner", "The owner of this object");

  group = createParameterGroupDesc( "Orders", "The order queues of the fleet");
  group->addParameter(obpT_Order_Queue, "Order Queue", "The queue of orders for this fleet");
  

}

OwnedObjectType::~OwnedObjectType(){
}

OwnedObject::OwnedObject():SpaceObject(){
}

OwnedObject::~OwnedObject(){
}

uint32_t OwnedObject::getOwner() const{
  return ((ReferenceObjectParam*)(obj->getParameter(2,1)))->getReferencedId();
}

void OwnedObject::setOwner(uint32_t no){
  ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferencedId(no);
  obj->touchModTime();
}



void OwnedObject::packExtraData(Frame * frame){
  SpaceObject::packExtraData(frame);
  
  ReferenceObjectParam* playerref = ((ReferenceObjectParam*)(obj->getParameter(2,1)));
  frame->packInt((playerref->getReferencedId() == 0) ? 0xffffffff : playerref->getReferencedId());

}


void OwnedObject::setupObject(){
  ObjectBehaviour::setupObject();
  ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferenceType(rst_Player);
  //something about the orderqueue?
}

