

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
