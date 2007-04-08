/*  SizeObjectParam baseclass
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

#include "sizeobjectparam.h"

SizeObjectParam::SizeObjectParam() : ObjectParameter(), size(0){
  type = obpT_Size;
}

SizeObjectParam::~SizeObjectParam(){

}


void SizeObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  f->packInt64(size);
}


bool SizeObjectParam::unpackModifyObjectFrame(Frame *f, unsigned int playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(8))
    return false;
  //size = f->unpackInt64();
  f->setUnpackOffset(f->getUnpackOffset() + 8);
  return true;
}

ObjectParameter* SizeObjectParam::clone() const{
  return new SizeObjectParam();
}

uint64_t SizeObjectParam::getSize() const{
  return size;
}

void SizeObjectParam::setSize(uint64_t ns){
  size = ns;
}

