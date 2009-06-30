/*  ObjectType base class
 *
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

#include <time.h>

#include "frame.h"

#include "objectinfo.h"

ObjectInfoData::ObjectInfoData() : type(0), name(), desc(){
}

ObjectInfoData::~ObjectInfoData(){
}

uint32_t ObjectInfoData::getType() const{
  return type;
}

std::string ObjectInfoData::getName() const{
  return name;
}

std::string ObjectInfoData::getDescription() const{
  return desc;
}

void ObjectInfoData::setType(uint32_t nt){
  type = nt;
  touchModTime();
}

void ObjectInfoData::setName(const std::string& nn){
  name = nn;
  touchModTime();
}

void ObjectInfoData::setDescription(const std::string& nd){
  desc = nd;
  touchModTime();
}

void ObjectInfoData::packFrame(Frame* frame){
  frame->packInt(type);
  frame->packString(name);
  if(frame->getVersion() >= fv0_4){
    frame->packString(desc);
  }
}

void ObjectInfoData::unpackModFrame(Frame* f){
  f->unpackInt(); // discard type
  std::string tn = f->unpackStdString();
  std::string td = f->unpackStdString();
  if(tn != name){
    tn = name;
    touchModTime();
  }
  if(td != desc){
    desc = td;
    touchModTime();
  }
}

bool ObjectInfoData::isDirty() const{
  return dirty;
}

void ObjectInfoData::setIsDirty(bool id){
  dirty = id;
}

void ObjectInfoData::touchModTime(){
  Modifiable::touchModTime();
  dirty = true;
}
