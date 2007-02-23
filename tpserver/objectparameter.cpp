/*  ObjectParameter baseclass
 *
 *  Copyright (C) 2007 Lee Begg and the Thousand Parsec Project
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

#include <stdlib.h>

#include "frame.h"

#include "objectparameter.h"

ObjectParameter::ObjectParameter() : OrderParameter(), object(0){
  type = opT_Object_ID;
}

ObjectParameter::~ObjectParameter(){

}


void ObjectParameter::packOrderFrame(Frame * f, uint32_t objID){
  f->packInt(object);
}

bool ObjectParameter::unpackFrame(Frame *f, unsigned int playerid){
  if(f->getDataLength() - f->getUnpackOffset() >= 4){
    object = f->unpackInt();
    return true;
  }else{
    return false;
  }
}

OrderParameter *ObjectParameter::clone() const{
  return new ObjectParameter();
}

uint32_t ObjectParameter::getObjectId() const{
  return object;
}

void ObjectParameter::setObjectId(uint32_t id){
  object = id;
}

