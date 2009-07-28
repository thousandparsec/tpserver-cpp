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
  return UINT32_NEG_ONE;
}

uint32_t ObjectTypeManager::addNewObjectType(ObjectType* od){
  od->setType(nextType);
  typeStore[nextType] = od;
  stringmap[od->getTypeName()] = nextType;
  nextType++;
  seqkey++;
  return nextType - 1;
}

IdModList ObjectTypeManager::getTypeModList(uint64_t fromtime) const {
  IdModList modlist;
  for(std::map<uint32_t, ObjectType*>::const_iterator itcurr = typeStore.begin();
      itcurr != typeStore.end(); ++itcurr){
    uint32_t otmodtime = itcurr->second->getModTime();
    if(fromtime == UINT64_NEG_ONE || otmodtime > fromtime){
      modlist[itcurr->first] = otmodtime;
    }
  }
  return modlist;
}

void ObjectTypeManager::doGetObjectDesc(uint32_t type, Frame* of){
  if(typeStore.find(type) != typeStore.end()){
    typeStore[type]->packObjectDescFrame(of);
  }else{
    of->createFailFrame(fec_NonExistant, "Object type does not exist");
  }
}
