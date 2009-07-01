/*  In Game Objects in the game universe
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

#include <string.h>
#include <cassert>

#include "frame.h"
#include "logging.h"
#include "game.h"
#include "objectmanager.h"
#include "playermanager.h"
#include "player.h"
#include "playerview.h"
#include "position3dobjectparam.h"
#include "velocity3dobjectparam.h"
#include "sizeobjectparam.h"
#include "orderqueueobjectparam.h"
#include "ordermanager.h"
#include "orderqueue.h"
#include "objectbehaviour.h"

#include "object.h"


IGObject::IGObject() : id(0xffffffff), turn(0), alive(true), type(0), info(new ObjectInfoData()), 
                   relationships(new ObjectRelationshipsData()), parameters(), behaviour(NULL){
}

IGObject::IGObject(const IGObject & rhs){
  WARNING("Object Copy Constructor: copying Object ID");
}

IGObject::~IGObject(){
  parameters.clear();
  if(behaviour != NULL){
    delete behaviour;
  }
}

IGObject& IGObject::operator=(const IGObject & rhs){
  //TODO
  WARNING("Object Assignment operator");
  return *this;
}

uint32_t IGObject::getID() const{
  return id;
}

uint32_t IGObject::getType() const{
  return type;
}

std::string IGObject::getName() const{
  return name;
}

std::string IGObject::getDescription() const{
  return desc;
}

bool IGObject::isAlive() const{
  return alive;
}

uint32_t IGObject::getTurn() const{
  return turn;
}

uint32_t IGObject::getParent() const{
  return relationships->getParent();
}

void IGObject::setID(uint32_t newid){
  touchModTime();
  id = newid;
}

void IGObject::setType(uint32_t newtype){
  type = newtype;
  touchModTime();
}

void IGObject::setName(const std::string &newname){
  name = newname;
  touchModTime();
}

void IGObject::setDescription(const std::string &newdesc){
  desc = newdesc;
  touchModTime();
}

void IGObject::setIsAlive(bool ia){
  alive = ia;
  touchModTime();
}

void IGObject::addToParent(uint32_t pid){
    relationships->setParent(pid);
    Game::getGame()->getObjectManager()->getObject(pid)->addContainedObject(id);
    Game::getGame()->getObjectManager()->doneWithObject(pid);
    touchModTime();
}

void IGObject::removeFromParent(){
  Game::getGame()->getObjectManager()->getObject(relationships->getParent())->removeContainedObject(id);
  Game::getGame()->getObjectManager()->doneWithObject(relationships->getParent());
  relationships->setParent(0);
  touchModTime();
}

int IGObject::getContainerType(){
  if(behaviour != NULL){
    return behaviour->getContainerType();
  }
  return 0;
}

std::set<uint32_t> IGObject::getContainedObjects(){
  return relationships->getChildren();
}

void IGObject::addContainedObject(uint32_t addObjectID){
  if(getContainerType() >= 1){
    //check that the new object is *really* inside the object

    //Game::getGame()->getObject(addObjectID)->parentid = id;
    touchModTime();
    relationships->addChild(addObjectID);
  }
}

void IGObject::removeContainedObject(uint32_t removeObjectID){
  // remove object
  touchModTime();
  relationships->removeChild(removeObjectID);
}

ObjectParameter* IGObject::getParameter(uint32_t groupnum, uint32_t paramnum) const{
  if(groupnum > 0 && groupnum < parameters.size() + 1){
    return parameters.find(groupnum)->second->getParameter(paramnum);
  }else{
    return NULL;
  }
}

void IGObject::setParameterGroup(const ObjectParameterGroupPtr &ng){
  parameters[ng->getGroupId()] = ng;
}

std::map<uint32_t, ObjectParameterGroupPtr> IGObject::getParameterGroups() const{
  return parameters;
}

ObjectBehaviour* IGObject::getObjectBehaviour() const{
  return behaviour;
}

void IGObject::setObjectBehaviour(ObjectBehaviour* nob){
  behaviour = nob;
  behaviour->setObject(this);
}

uint64_t IGObject::getModTime() const{
  uint64_t maxmodtime = Modifiable::getModTime();
  return maxmodtime;
}

bool IGObject::isDirty() const{
  bool dirtyparams = false; //TODO fix
  return isDirty() || info->isDirty() || relationships->isDirty() || dirtyparams;
}

void IGObject::setParent(uint32_t pid){
  relationships->setParent(pid);
}

void IGObject::setIsDirty(bool id){
  Modifiable::setIsDirty(id);
  info->setIsDirty(id);
  relationships->setIsDirty(id);
}

ObjectParameter* IGObject::getParameterByType(uint32_t ptype) const{
  for(std::map<uint32_t, ObjectParameterGroupPtr>::const_iterator itcurr = parameters.begin(); itcurr != parameters.end();
      ++itcurr){
    ObjectParameterGroupData::ParameterList params = (itcurr->second)->getParameters();
    for(ObjectParameterGroupData::ParameterList ::iterator opitcurr = params.begin(); opitcurr != params.end();
        ++opitcurr){
      if((*opitcurr)->getType() == ptype){
        return (*opitcurr);
      }
    }
  }
  return NULL;
}
