/*  ObjectManager class
 *
 *  Copyright (C) 2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include "persistence.h"
#include "game.h"
#include "object.h"
#include "ordermanager.h"
#include "sizeobjectparam.h"
#include "position3dobjectparam.h"
#include "objectbehaviour.h"

#include "objectmanager.h"

ObjectManager::ObjectManager() : nextid(0){
}

ObjectManager::~ObjectManager(){
  // sync to persistence
}

void ObjectManager::init(){
  IdSet vis(Game::getGame()->getPersistence()->getObjectIds());
  for(IdSet::const_iterator itcurr = vis.begin(); itcurr != vis.end(); ++itcurr){
    objects[*itcurr] = NULL;
  }

  nextid = Game::getGame()->getPersistence()->getMaxObjectId();
  if(nextid != 0)
    nextid++;
}

IGObject* ObjectManager::createNewObject(){
  return new IGObject(nextid++);
}

void ObjectManager::addObject(IGObject* obj){
  objects[obj->getID()] = obj;
  Game::getGame()->getPersistence()->saveObject(obj);
}

void ObjectManager::discardNewObject(IGObject* obj){
  if(obj->getID() == nextid - 1){
    nextid = obj->getID();
  }
  ObjectBehaviour* behaviour = obj->getObjectBehaviour();
  if(behaviour != NULL){
    behaviour->signalRemoval();
  }
  delete obj;
}

IGObject *ObjectManager::getObject(uint32_t id){
  IGObject *rtn = NULL;

  std::map < uint32_t, IGObject * >::iterator obj = objects.find(id);
  if (obj != objects.end()) {
    rtn = (*obj).second;
  }
  if(rtn == NULL){
    rtn = Game::getGame()->getPersistence()->retrieveObject(id);
    if(rtn != NULL){
      objects[id] = rtn;
    }
  }
  return rtn;
  //may need more work
}

void ObjectManager::doneWithObject(uint32_t id){
  if(objects[id]->isDirty()){
    Game::getGame()->getPersistence()->saveObject(objects[id]);
  }
}

void ObjectManager::scheduleRemoveObject(uint32_t id){
  scheduleRemove.insert(id);
  IGObject* ob = objects[id];
  ob->setIsAlive(false);
  Game::getGame()->getPersistence()->saveObject(ob);
}

void ObjectManager::clearRemovedObjects(){
  for(IdSet::iterator itrm = scheduleRemove.begin(); itrm != scheduleRemove.end(); ++itrm){
    objects[*itrm]->removeFromParent();
    objects[*itrm]->getObjectBehaviour()->signalRemoval();
    delete objects[*itrm];
    objects.erase(*itrm);
  }
  scheduleRemove.clear();
}

IdSet ObjectManager::getObjectsByPos(const Vector3d & pos, uint64_t r){
  IdSet oblist;

  for(std::map<uint32_t, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
    IGObject* ob = itcurr->second;
    if(ob == NULL){
      ob = Game::getGame()->getPersistence()->retrieveObject(itcurr->first);
      if(ob != NULL){
        itcurr->second = ob;
      }
    }
    if(ob != NULL){
      SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(ob->getParameterByType(obpT_Size));
      Position3dObjectParam * obpos = dynamic_cast<Position3dObjectParam*>(ob->getParameterByType(obpT_Position_3D));
      if(size != NULL && obpos != NULL){
        uint64_t br = size->getSize() / 2;
        uint64_t diff = obpos->getPosition().getDistance(pos); /*- r - br;*/
        if(diff <=  r + br)
          oblist.insert(itcurr->first);
      }
    }
  }

  return oblist;
}


IdSet ObjectManager::getAllIds(){
  IdSet vis;
  for(std::map<uint32_t, IGObject*>::const_iterator itid = objects.begin();
      itid != objects.end(); ++itid){
    IGObject* obj = itid->second;
    if(obj == NULL){
      obj = Game::getGame()->getPersistence()->retrieveObject(itid->first);
      if(obj != NULL){
        objects[itid->first] = obj;
      }
    }
    if(obj != NULL){
      if(obj->isAlive()){
        vis.insert(itid->first);
      }
    }
  }

  return vis;
}

uint32_t ObjectManager::getNumObjects() const{
  return objects.size();
}
