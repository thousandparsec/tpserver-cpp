/*  ObjectDataManager for managing ObjectData objects
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include "objecttype.h"
#include "frame.h"
#include "object.h"

#include "objecttypemanager.h"

ObjectTypeManager::ObjectTypeManager() : typeStore(), stringmap(), nextType(0), seqkey(0){
}

ObjectTypeManager::~ObjectTypeManager(){
   for(std::map<uint32_t, ObjectType*>::iterator itcurr = typeStore.begin(); itcurr != typeStore.end(); ++itcurr){
    delete itcurr->second;
  }
}

void ObjectTypeManager::setupObject(IGObject* obj, uint32_t type){
  if(checkValid(type)){
    ObjectType* prototype = typeStore[type];
    obj->setType(type);
    prototype->setupObject(obj);
  }else{
    //TODO throw exception?
  }
}

bool ObjectTypeManager::checkValid(uint32_t type){
  return (0 <= type && type <= nextType - 1);
}

uint32_t ObjectTypeManager::getObjectTypeByName(const std::string& name) const{
  if(stringmap.find(name) != stringmap.end()){
    return stringmap.find(name)->second;
  }
  //TODO throw exception?
  return 0xffffffff;
}

uint32_t ObjectTypeManager::addNewObjectType(ObjectType* od){
  od->setType(nextType);
  typeStore[nextType] = od;
  stringmap[od->getTypeName()] = nextType;
  nextType++;
  seqkey++;
  return nextType - 1;
}

void ObjectTypeManager::doGetObjectTypes(Frame* frame, Frame* of){
  uint32_t lseqkey = frame->unpackInt();
  if(lseqkey == 0xffffffff){
    //start new seqkey
    lseqkey = seqkey;
  }

  uint32_t start = frame->unpackInt();
  uint32_t num = frame->unpackInt();
  uint64_t fromtime = 0xffffffffffffffffULL;
  if(frame->getVersion() >= fv0_4){
    fromtime = frame->unpackInt64();
  }

  if(lseqkey != seqkey){
    of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
    return;
  }

  std::map<uint32_t, uint64_t> modlist;
  for(std::map<uint32_t, ObjectType*>::iterator itcurr = typeStore.begin();
      itcurr != typeStore.end(); ++itcurr){
    uint32_t otmodtime = itcurr->second->getModTime();
    if(fromtime == 0xffffffffffffffffULL || otmodtime < fromtime){
      modlist[itcurr->first] = otmodtime;
    }
  }
  
  if(start > modlist.size()){
    of->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  
  if(num > modlist.size() - start){
      num = modlist.size() - start;
  }
  
  if(num > 87378 + ((frame->getVersion() < fv0_4)? 1 : 0)){
    of->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  of->setType(ft04_ObjectTypes_List);
  of->packInt(lseqkey);
  of->packInt(modlist.size() - start - num);
  of->packInt(num);
  std::map<uint32_t, uint64_t>::iterator itcurr = modlist.begin();
  advance(itcurr, start);
  for(uint32_t i = 0; i < num; i++){
    of->packInt(itcurr->first);
    of->packInt64(itcurr->second);
    ++itcurr;
  }
  
  if(of->getVersion() >= fv0_4){
    of->packInt64(fromtime);
  }
  
}

void ObjectTypeManager::doGetObjectDesc(uint32_t type, Frame* of){
  if(typeStore.find(type) != typeStore.end()){
    typeStore[type]->packObjectDescFrame(of);
  }else{
    of->createFailFrame(fec_NonExistant, "Object type does not exist");
  }
}
