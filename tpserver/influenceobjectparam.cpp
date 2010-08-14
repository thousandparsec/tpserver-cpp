/*  InfluenceObjectParam baseclass
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

#include "influenceobjectparam.h"

InfluenceObjectParam::InfluenceObjectParam() : ObjectParameter(), value(1e12){
  type = obpT_Influence;
}

InfluenceObjectParam::~InfluenceObjectParam(){

}


void InfluenceObjectParam::packObjectFrame(OutputFrame::Ptr f, uint32_t objID){
  f->packInt64(value);
}


bool InfluenceObjectParam::unpackModifyObjectFrame(InputFrame::Ptr f, uint32_t playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(8))
    return false;
  //value = f->unpackInt();
  f->advance(8);
  return true;
}

ObjectParameter* InfluenceObjectParam::clone() const{
  return new InfluenceObjectParam();
}

uint32_t InfluenceObjectParam::getValue() const{
  return value;
}

void InfluenceObjectParam::setValue(uint32_t nv){
  value = nv;
}

