/*  ReferenceObjectParam baseclass
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

#include "frame.h"

#include "referenceobjectparam.h"

ReferenceObjectParam::ReferenceObjectParam() : ObjectParameter(), reftype(0), refid(0){
  type = obpT_Reference;
}

ReferenceObjectParam::~ReferenceObjectParam(){

}


void ReferenceObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  f->packInt(reftype);
  f->packInt(refid);
}


bool ReferenceObjectParam::unpackModifyObjectFrame(Frame *f, uint32_t playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(8))
    return false;
  //reftype = f->unpackInt();
  //refid = f->unpackInt();
  f->advance(8);
  return true;
}

ObjectParameter* ReferenceObjectParam::clone() const{
  return new ReferenceObjectParam();
}

int32_t ReferenceObjectParam::getReferenceType() const{
  return reftype;
}

void ReferenceObjectParam::setReferenceType(int32_t nt){
  reftype = nt;
}

uint32_t ReferenceObjectParam::getReferencedId() const{
  return refid;
}

void ReferenceObjectParam::setReferencedId(uint32_t ni){
  refid = ni;
}

