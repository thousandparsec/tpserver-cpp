/*  ObjectDataManager for managing ObjectData objects
 *
 *  Copyright (C) 2003-2004  Lee Begg and the Thousand Parsec Project
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
#include "universe.h"
#include "emptyobject.h"
#include "planet.h"
#include "fleet.h"

#include "objectdatamanager.h"

ObjectDataManager::ObjectDataManager(){
  nextType = 1000;

  prototypeStore[obT_Universe] = new Universe();
  prototypeStore[obT_Galaxy] = new EmptyObject();
  prototypeStore[obT_Star_System] = new EmptyObject();
  prototypeStore[obT_Planet] = new Planet();
  prototypeStore[obT_Fleet] = new Fleet();
}

ObjectDataManager::~ObjectDataManager(){
   for(std::map<int, ObjectData*>::iterator itcurr = prototypeStore.begin(); itcurr != prototypeStore.end(); ++itcurr){
    delete itcurr->second;
  }
}

ObjectData* ObjectDataManager::createObjectData(int type){
  ObjectData* prototype = prototypeStore[type];
  if(prototype != NULL){
    return prototype->clone();
  }
  return NULL;
}

bool ObjectDataManager::checkValid(int type){
  return ((obT_Invalid < type && type < obT_Max) || (1000 <= type && type <= nextType - 1));
}

int ObjectDataManager::addNewObjectType(ObjectData* od){
  prototypeStore[nextType++] = od;
  return nextType - 1;
}
