/*  OwnedObject baseclass for ObjectData classes that are owned by players
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

#include "frame.h"
#include "objectparametergroup.h"
#include "refsys.h"
#include "referenceobjectparam.h"

#include "ownedobject.h"

OwnedObject::OwnedObject() : ObjectData(){
  playerref = new ReferenceObjectParam();
  playerref->setName("Owner");
  playerref->setDescription("The owner of this object");
  playerref->setReferenceType(rst_Player);
  ObjectParameterGroup * group = new ObjectParameterGroup();
  group->setGroupId(1);
  group->setName("Ownership");
  group->setDescription("The ownership of this object");
  group->addParameter(playerref);
  paramgroups.push_back(group);
  touchModTime();
}

void OwnedObject::packExtraData(Frame * frame){
  frame->packInt((playerref->getReferencedId() == 0) ? 0xffffffff : playerref->getReferencedId());
}

void OwnedObject::setOwner(int player){
  playerref->setReferencedId(player);
  touchModTime();
}

int OwnedObject::getOwner(){
  return playerref->getReferencedId();
}
