/*  ObjectDataManager for managing ObjectData objects
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

#include "objectdata.h"
#include "frame.h"

#include "objectdatamanager.h"

ObjectDataManager::ObjectDataManager() : seqkey(0){
  nextType = 0;
}

ObjectDataManager::~ObjectDataManager(){
   for(std::map<uint32_t, ObjectData*>::iterator itcurr = prototypeStore.begin(); itcurr != prototypeStore.end(); ++itcurr){
    delete itcurr->second;
  }
}

ObjectData* ObjectDataManager::createObjectData(uint32_t type){
  ObjectData* prototype = prototypeStore[type];
  if(prototype != NULL){
    return prototype->clone();
  }
  return NULL;
}

bool ObjectDataManager::checkValid(uint32_t type){
  return (0 <= type && type <= nextType - 1);
}

uint32_t ObjectDataManager::getObjectTypeByName(const std::string& name) const{
  if(stringmap.find(name) != stringmap.end()){
    return stringmap.find(name)->second;
  }
  return 0xffffffff;
}

int ObjectDataManager::addNewObjectType(ObjectData* od){
  prototypeStore[nextType] = od;
  stringmap[od->getTypeName()] = nextType;
  nextType++;
  seqkey++;
  return nextType - 1;
}

void ObjectDataManager::doGetObjectTypes(Frame* frame, Frame* of){
  unsigned int lseqkey = frame->unpackInt();
  if(lseqkey == 0xffffffff){
    //start new seqkey
    lseqkey = seqkey;
  }

  unsigned int start = frame->unpackInt();
  unsigned int num = frame->unpackInt();

  if(lseqkey != seqkey){
    of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
    return;
  }

  unsigned int num_remain;
  if(num == 0xffffffff || start + num > prototypeStore.size()){
    num = prototypeStore.size() - start;
    num_remain = 0;
  }else{
    num_remain = prototypeStore.size() - start - num;
  }

  of->setType(ft04_ObjectTypes_List);
  of->packInt(lseqkey);
  of->packInt(num_remain);
  of->packInt(num);
  std::map<uint32_t, ObjectData*>::iterator itcurr = prototypeStore.begin();
  advance(itcurr, start);
  for(unsigned int i = 0; i < num; i++){
    of->packInt(itcurr->first);
    of->packInt64(itcurr->second->getModTime());
    ++itcurr;
  }
}

void ObjectDataManager::doGetObjectDesc(uint32_t type, Frame* of){
  if(prototypeStore.find(type) != prototypeStore.end()){
    prototypeStore[type]->packObjectDescFrame(of);
  }else{
    of->createFailFrame(fec_NonExistant, "Object type does not exist");
  }
}
