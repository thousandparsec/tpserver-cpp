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
#include <boost/bind.hpp>

#include "object.h"


IGObject::IGObject(uint32_t newid) : ProtocolObject(ft02_Object,newid), turn(0), alive(true), type(0), parentid(0),
                   behaviour(NULL){
}

IGObject::~IGObject(){
  parameters.clear();
  if(behaviour != NULL){
    delete behaviour;
  }
}

uint32_t IGObject::getID() const{
  return id;
}

uint32_t IGObject::getType() const{
  return type;
}


bool IGObject::isAlive() const{
  return alive;
}

uint32_t IGObject::getTurn() const{
  return turn;
}

uint32_t IGObject::getParent() const{
  return parentid;
}

void IGObject::setType(uint32_t newtype){
  type = newtype;
  touchModTime();
}

void IGObject::setName(const std::string &newname){
  ProtocolObject::setName(newname);
  touchModTime();
}

void IGObject::setDescription(const std::string &newdesc){
  ProtocolObject::setDescription(newdesc);
  touchModTime();
}

void IGObject::setIsAlive(bool ia){
  alive = ia;
  touchModTime();
}

void IGObject::addToParent(uint32_t pid){
    setParent( pid );
    Game::getGame()->getObjectManager()->getObject(pid)->addContainedObject(id);
    Game::getGame()->getObjectManager()->doneWithObject(pid);
    touchModTime();
}

void IGObject::removeFromParent(){
  Game::getGame()->getObjectManager()->getObject(parentid)->removeContainedObject(id);
  Game::getGame()->getObjectManager()->doneWithObject(parentid);
  setParent(0);
  touchModTime();
}

int IGObject::getContainerType(){
  if(behaviour != NULL){
    return behaviour->getContainerType();
  }
  return 0;
}

IdSet IGObject::getContainedObjects(){
  return children;
}

void IGObject::addContainedObject(uint32_t addObjectID){
  if(getContainerType() >= 1){
    //check that the new object is *really* inside the object

    //Game::getGame()->getObject(addObjectID)->parentid = id;
    touchModTime();
    setIsDirty( children.insert(addObjectID).second );
  }
}

void IGObject::removeContainedObject(uint32_t removeObjectID){
  // remove object
  setIsDirty( (children.erase(removeObjectID) != 0) );
  touchModTime();
}

ObjectParameter* IGObject::getParameter(uint32_t groupnum, uint32_t paramnum) const{
  if(groupnum > 0 && groupnum < parameters.size() + 1){
    return parameters.find(groupnum)->second->getParameter(paramnum);
  }else{
    return NULL;
  }
}

void IGObject::setParameterGroup(const ObjectParameterGroup::Ptr &ng){
  parameters[ng->getGroupId()] = ng;
}

ObjectParameterGroup::Map IGObject::getParameterGroups() const{
  return parameters;
}

ObjectBehaviour* IGObject::getObjectBehaviour() const{
  return behaviour;
}

void IGObject::setObjectBehaviour(ObjectBehaviour* nob){
  behaviour = nob;
  behaviour->setObject(this);
}

void IGObject::setParent(uint32_t pid){
  setIsDirty( pid != parentid );
  parentid = pid;
}

ObjectParameter* IGObject::getParameterByType(uint32_t ptype) const{
  for(ObjectParameterGroup::Map::const_iterator itcurr = parameters.begin(); itcurr != parameters.end();
      ++itcurr){
    ObjectParameterGroup::ParameterList params = (itcurr->second)->getParameters();

    ObjectParameterGroup::ParameterList::iterator opitcurr = std::find_if( params.begin(), params.end(), boost::bind( &ObjectParameter::getType, _1 ) == ptype );
    if ( opitcurr != params.end() ) {
      return (*opitcurr );
    }
  }
  return NULL;
}
