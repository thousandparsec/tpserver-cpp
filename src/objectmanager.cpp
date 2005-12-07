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

#include "objectmanager.h"

ObjectManager::ObjectManager() : nextid(0){
}

ObjectManager::~ObjectManager(){
    // sync to persistence
}

IGObject* ObjectManager::createNewObject(){
    if(nextid == 0){
        nextid = Game::getGame()->getPersistence()->getMaxObjectId();
        if(nextid != 0)
            nextid++;
    }
    IGObject* obj = new IGObject();
    obj->setID(nextid++);
    return obj;
}

void ObjectManager::addObject(IGObject* obj){
    objects[obj->getID()] = obj;
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
        }
    }
    return rtn;
    //may need more work
}

void ObjectManager::doneWithObject(uint32_t id){
}

void ObjectManager::scheduleRemoveObject(uint32_t id){
    scheduleRemove.insert(id);
}

void ObjectManager::clearRemovedObjects(){
    for(std::set<unsigned int>::iterator itrm = scheduleRemove.begin(); itrm != scheduleRemove.end(); ++itrm){
        objects[*itrm]->removeFromParent();
        Game::getGame()->getPersistence()->removeObject(*itrm);
        delete objects[*itrm];
        objects.erase(*itrm);
    }
    scheduleRemove.clear();
}

std::set<uint32_t> ObjectManager::getObjectsByPos(const Vector3d & pos, uint64_t r){
    std::set <uint32_t> oblist;

    for(std::map<uint32_t, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        uint64_t br = itcurr->second->getSize() / 2;
        uint64_t diff = itcurr->second->getPosition().getDistance(pos); /*- r - br;*/
        if(diff <=  r + br)
        oblist.insert(itcurr->first);
    
    }
    
    return oblist;
}

std::set<uint32_t> ObjectManager::getContainerByPos(const Vector3d & pos){
    std::set<uint32_t> oblist;

    for(std::map<uint32_t, IGObject *>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr){
        if(itcurr->second->getContainerType() >= 1){
        uint64_t br = itcurr->second->getSize() / 2 + itcurr->second->getSize() % 2;
        
        //long long diff = itcurr->second->getPosition().getDistanceSq(pos) - br * br;
        if(itcurr->second->getPosition().getDistance(pos) <= br)
            oblist.insert(itcurr->first);
        }
    }
    
    return oblist;
}

std::set<uint32_t> ObjectManager::getAllIds(){
    std::set<unsigned int> vis;
    for(std::map<uint32_t, IGObject*>::const_iterator itid = objects.begin();
        itid != objects.end(); ++itid){
        vis.insert(itid->first);
    }
    return vis;
}
