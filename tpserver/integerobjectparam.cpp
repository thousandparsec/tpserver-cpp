/*  IntegerObjectParam baseclass
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

#include "integerobjectparam.h"

IntegerObjectParam::IntegerObjectParam() : ObjectParameter(), value(0){
  type = obpT_Integer;
}

IntegerObjectParam::~IntegerObjectParam(){

}


void IntegerObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  f->packInt(value);
}


bool IntegerObjectParam::unpackModifyObjectFrame(Frame *f, uint32_t playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(4))
    return false;
  //value = f->unpackInt();
  f->setUnpackOffset(f->getUnpackOffset() + 4);
  return true;
}

ObjectParameter* IntegerObjectParam::clone() const{
  return new IntegerObjectParam();
}

uint32_t IntegerObjectParam::getValue() const{
  return value;
}

void IntegerObjectParam::setValue(uint32_t nv){
  value = nv;
}

