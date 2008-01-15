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

#include "frame.h"

#include "objectinfo.h"

ObjectInfoData::ObjectInfoData() : ref(0), type(0), name(), desc(){
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
}

void ObjectInfoData::setName(const std::string& nn){
  name = nn;
}

void ObjectInfoData::setDescription(const std::string& nd){
  desc = nd;
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
  }
  if(td != desc){
    desc = td;
  }
}

