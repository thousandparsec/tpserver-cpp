/*  ResourceListObjectParam baseclass
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

#include "resourcelistobjectparam.h"

ResourceListObjectParam::ResourceListObjectParam() : ObjectParameter(), resources(){
  type = obpT_Resource_List;
}

ResourceListObjectParam::~ResourceListObjectParam(){

}


void ResourceListObjectParam::packObjectFrame(Frame * f, uint32_t objID){
  f->packInt(resources.size());
  for(ResourceMap::const_iterator itcurr = resources.begin();
      itcurr != resources.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second.first);
    f->packInt(itcurr->second.second);
    f->packInt(0);
  }
}


bool ResourceListObjectParam::unpackModifyObjectFrame(Frame *f, unsigned int playerid){
  // all fields are read only
  if(!f->isEnoughRemaining(4))
    return false;
  uint32_t reflistsize = f->unpackInt();
  if(!f->isEnoughRemaining(16 * reflistsize))
    return false;
  //list of reflistsize
  // restype = f->unpackInt();
  // surface = f->unpackInt();
  // minable = f->unpackInt();
  // unminable = f->unpackInt();
  f->setUnpackOffset(f->getUnpackOffset() + 16 * reflistsize);
  return true;
}

ObjectParameter* ResourceListObjectParam::clone() const{
  return new ResourceListObjectParam();
}

ResourceListObjectParam::ResourceMap ResourceListObjectParam::getResources() const{
  return resources;
}

void ResourceListObjectParam::setResources(ResourceMap nt){
  resources = nt;
}
