/*  ObjectManager class
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include "objectmanager.h"

ObjectManager::ObjectManager() : nextid(0){
}

ObjectManager::~ObjectManager(){
    // sync to persistence
}

void ObjectManager::init(){
    std::set<uint32_t> vis(Game::getGame()->getPersistence()->getObjectIds());
    for(std::set<uint32_t>::const_iterator itcurr = vis.begin(); itcurr != vis.end(); ++itcurr){
        objects[*itcurr] = NULL;
    }

    nextid = Game::getGame()->getPersistence()->getMaxObjectId();
    if(nextid != 0)
        nextid++;
}

IGObject* ObjectManager::createNewObject(){
    IGObject* obj = new IGObject();
    obj->setID(nextid++);
    return obj;
}

void ObjectManager::addObject(IGObject* obj){
    objects[obj->getID()] = obj;
    objmtime[obj->getID()] = obj->getModTime();
    Game::getGame()->getPersistence()->saveObject(obj);
}

void ObjectManager::discardNewObject(IGObject* obj){
    if(obj->getID() == nextid - 1){
        nextid = obj->getID();
    }
    delete obj;
}

IGObject *ObjectManager::getObject(uint32_t id){
     IGObject *rtn = NULL;
   
    std::map < unsigned int, IGObject * >::iterator obj = objects.find(id);
    if (obj != objects.end()) {
        rtn = (*obj).second;
    }
    if(rtn == NULL){
        rtn = Game::getGame()->getPersistence()->retrieveObject(id);
        if(rtn != NULL){
            objects[id] = rtn;
            objmtime[id] = rtn->getModTime();
        }
    }
    return rtn;
    //may need more work
}

void ObjectManager::doneWithObject(uint32_t id){
    if(objmtime[id] >= ((uint64_t)(time(NULL) - 2)) || 
            (uint64_t)(objects[id]->getModTime()) >= objmtime[id]){
        objmtime[id] = objects[id]->getModTime();
        Game::getGame()->getPersistence()->updateObject(objects[id]);
    }
}

void ObjectManager::scheduleRemoveObject(uint32_t id){
    scheduleRemove.insert(id);
}

void ObjectManager::clearRemovedObjects(){
    for(std::set<unsigned int>::iterator itrm = scheduleRemove.begin(); itrm != scheduleRemove.end(); ++itrm){
        objects[*itrm]->removeFromParent();
        Game::getGame()->getOrderManager()->removeAllOrders(*itrm);
        Game::getGame()->getPersistence()->removeObject(*itrm);
        delete objects[*itrm];
        objects.erase(*itrm);
        objmtime.erase(*itrm);
    }
    scheduleRemove.clear();
}

std::set<uint32_t> ObjectManager::getObjectsByPos(const Vector3d & pos, uint64_t r){
    std::set <uint32_t> oblist;

    for(std::map<uint32_t, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject* ob = itcurr->second;
        if(ob == NULL){
            ob = Game::getGame()->getPersistence()->retrieveObject(itcurr->first);
            if(ob != NULL){
                itcurr->second = ob;
                objmtime[itcurr->first] = ob->getModTime();
            }
        }
        if(ob != NULL){
            uint64_t br = ob->getSize() / 2;
            uint64_t diff = ob->getPosition().getDistance(pos); /*- r - br;*/
            if(diff <=  r + br)
            oblist.insert(itcurr->first);
        }
    }
    
    return oblist;
}

std::set<uint32_t> ObjectManager::getContainerByPos(const Vector3d & pos){
    std::set<uint32_t> oblist;

    for(std::map<uint32_t, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr){
        IGObject* ob = itcurr->second;
        if(ob == NULL){
            ob = Game::getGame()->getPersistence()->retrieveObject(itcurr->first);
            if(ob != NULL){
                itcurr->second = ob;
                objmtime[itcurr->first] = ob->getModTime();
            }
        }
        if(ob != NULL){
            if(ob->getContainerType() >= 1){
            uint64_t br = ob->getSize() / 2 + itcurr->second->getSize() % 2;
            
            //long long diff = itcurr->second->getPosition().getDistanceSq(pos) - br * br;
            if(ob->getPosition().getDistance(pos) <= br)
                oblist.insert(itcurr->first);
            }
        }
    }
    return oblist;
}

std::set<uint32_t> ObjectManager::getAllIds(){
    std::set<uint32_t> vis;
    for(std::map<uint32_t, IGObject*>::const_iterator itid = objects.begin();
        itid != objects.end(); ++itid){
        vis.insert(itid->first);
    }

    return vis;
}

uint32_t ObjectManager::getNumObjects() const{
  return objects.size();
}
