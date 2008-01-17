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
#include "objectdata.h"
#include "objectdatamanager.h"
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


IGObject::IGObject() : id(0xffffffff), turn(0), alive(true), info(), relationships(), parameters(),
                   behaviour(NULL){
  touchModTime();
}

IGObject::IGObject(const IGObject & rhs){
  Logger::getLogger()->warning("Object Copy Constructor: copying Object ID");
}

IGObject::~IGObject(){
  parameters.clear();
  if(behaviour != NULL){
    delete behaviour;
  }
}

IGObject& IGObject::operator=(const IGObject & rhs){
  //TODO
  Logger::getLogger()->warning("Object Assignment operator");
  return *this;
}

uint32_t IGObject::getID() const{
  return id;
}

uint32_t IGObject::getType() const{
  return info->getType();
}

std::string IGObject::getName() const{
  return info->getName();
}

std::string IGObject::getDescription() const{
  return info->getDescription();
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
  info->setType(newtype);
  touchModTime();
}

void IGObject::setName(const std::string &newname){
  info->setName(newname);
  touchModTime();
}

void IGObject::setDescription(const std::string &newdesc){
  info->setDescription(newdesc);
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

void IGObject::removeContainedObject(unsigned int removeObjectID){
  // remove object
  touchModTime();
  relationships->removeChild(removeObjectID);
}

void IGObject::createFrame(Frame * frame, uint32_t playerid){
  
  frame->setType(ft02_Object);
  frame->packInt(id);
  info->packFrame(frame);
  
  if(frame->getVersion() >= fv0_4){
    // should be moved to ObjectRelationshipsData::packFrame once TP03 support is removed
    frame->packInt(relationships->getParent());
  }else{
    //pre tp04
   
    SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(getParameterByType(obpT_Size));
    if(size != NULL){
      frame->packInt64(size->getSize());
    }else{
      frame->packInt64(0);
    }
  
    Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(getParameterByType(obpT_Position_3D));
    if(pos != NULL){
      pos->getPosition().pack(frame);
    }else{
      Vector3d(0,0,0).pack(frame);
    }
  
    Velocity3dObjectParam * vel = dynamic_cast<Velocity3dObjectParam*>(getParameterByType(obpT_Velocity));
    if(vel != NULL){
      vel->getVelocity().pack(frame);
    }else{
      Vector3d(0,0,0).pack(frame);
    }
  
  }
 
  relationships->packFrame(frame, playerid);

  if(frame->getVersion() <= fv0_3){
  
    
    OrderQueueObjectParam* oq = dynamic_cast<OrderQueueObjectParam*>(getParameterByType(obpT_Order_Queue));
    if(oq != NULL){
      OrderQueue* queue = Game::getGame()->getOrderManager()->getOrderQueue(oq->getQueueId());
      if(queue->isOwner(playerid)){
        std::set<uint32_t> allowedtypes = queue->getAllowedOrderTypes();
        frame->packInt(allowedtypes.size());
        for(std::set<uint32_t>::iterator itcurr = allowedtypes.begin(); itcurr != allowedtypes.end();
            ++itcurr){
          frame->packInt(*itcurr);
        }
        frame->packInt(queue->getNumberOrders());
      }else{
        frame->packInt(0);
        frame->packInt(0);
      }
    }else{
      frame->packInt(0);
      frame->packInt(0);
    }
      
  }
  //padding
  if(frame->getVersion() >= fv0_3){
    frame->packInt64(getModTime());
    frame->packInt(0);
    frame->packInt(0);
    if(frame->getVersion() >= fv0_4){
      frame->packInt(0);
      frame->packInt(0);
    }
  }else{
    frame->packInt(0);
    frame->packInt(0);
    frame->packInt(0);
    frame->packInt(0);
  }
  if(frame->getVersion() >= fv0_4){
    for(std::map<uint32_t, ObjectParameterGroupPtr>::iterator itcurr = parameters.begin(); itcurr != parameters.end();
      ++itcurr){
    (itcurr->second)->packObjectFrame(frame, playerid);
  }
  }else{
    if(behaviour != NULL){
      behaviour->packExtraData(frame);
    }
  }

}

ObjectBehaviour* IGObject::getObjectBehaviour() const{
  return behaviour;
}

void IGObject::touchModTime(){
  modtime = time(NULL);
}

uint64_t IGObject::getModTime() const{
  uint64_t maxmodtime = modtime;
  if(info->getModTime() > maxmodtime){
    maxmodtime = info->getModTime();
  }
  
  return maxmodtime;
}


void IGObject::setParent(uint32_t pid){
  relationships->setParent(pid);
}

void IGObject::setModTime(uint64_t time){
  modtime = time;
}

ObjectParameter* IGObject::getParameterByType(uint32_t ptype) const{
  for(std::map<uint32_t, ObjectParameterGroupPtr>::const_iterator itcurr = parameters.begin(); itcurr != parameters.end();
      ++itcurr){
    std::list<ObjectParameter*> params = (itcurr->second)->getParameters();
    for(std::list<ObjectParameter*>::iterator opitcurr = params.begin(); opitcurr != params.end();
        ++opitcurr){
      if((*opitcurr)->getType() == ptype){
        return (*opitcurr);
      }
    }
  }
  return NULL;
}
