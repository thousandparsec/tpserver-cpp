/*  RefQuantityListObjectParam baseclass
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

#include "refquantitylistobjectparam.h"

RefQuantityListObjectParam::RefQuantityListObjectParam() : ObjectParameter(), refquant(){
  type = obpT_Reference_Quantity_List;
}

RefQuantityListObjectParam::~RefQuantityListObjectParam(){

}


void RefQuantityListObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  f->packInt(refquant.size());
  for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = refquant.begin();
      itcurr != refquant.end(); ++itcurr){
    f->packInt(itcurr->first.first);
    f->packInt(itcurr->first.second);
    f->packInt(itcurr->second);
  }
}


bool RefQuantityListObjectParam::unpackModifyObjectFrame(Frame *f, unsigned int playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(4))
    return false;
  uint32_t reflistsize = f->unpackInt();
  if(!f->isEnoughRemaining(12 * reflistsize))
    return false;
  //list of reflistsize
  // reftype = f->unpackInt();
  // refid = f->unpackInt();
  // quantity = f->unpackInt();
  f->setUnpackOffset(f->getUnpackOffset() + 12 * reflistsize);
  return true;
}

ObjectParameter* RefQuantityListObjectParam::clone() const{
  return new RefQuantityListObjectParam();
}

std::map<std::pair<int32_t, uint32_t>, uint32_t> RefQuantityListObjectParam::getRefQuantityList() const{
  return refquant;
}

void RefQuantityListObjectParam::setRefQuantityList(std::map<std::pair<int32_t, uint32_t>, uint32_t> nt){
  refquant = nt;
}
