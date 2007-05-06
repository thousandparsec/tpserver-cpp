/*  In Game Objects in the game universe
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include "position3dobjectparam.h"
#include "velocity3dobjectparam.h"
#include "sizeobjectparam.h"
#include "orderqueueobjectparam.h"
#include "ordermanager.h"
#include "orderqueue.h"

#include "object.h"

Game *IGObject::myGame = NULL;

IGObject::IGObject() : name(), description()
{
	id = 0xffffffff;
	parentid = 0;
	if (myGame == NULL) {
		myGame = Game::getGame();
	}
	myObjectData = NULL;
}

IGObject::IGObject(IGObject & rhs){
  Logger::getLogger()->warning("Object Copy Constructor: copying Object ID");
}

IGObject::~IGObject()
{
	children.clear();
	if(myObjectData != NULL){
	  delete myObjectData;
	}
}

IGObject IGObject::operator=(IGObject & rhs)
{
	//TODO
        Logger::getLogger()->warning("Object Assignment operator");
	return *this;
}

unsigned int IGObject::getID()
{
	return id;
}

unsigned int IGObject::getType()
{
	return type;
}

std::string IGObject::getName(){
  return name;
}

bool IGObject::setID(unsigned int newid)
{
	//check to see if id is use
	//if it is, then return false
	//else
  touchModTime();
	id = newid;
	return true;
}

unsigned int IGObject::getParent(){
  return parentid;
}

void IGObject::setType(unsigned int newtype)
{
	type = newtype;

	if(myObjectData != NULL)
	  delete myObjectData;

	myObjectData = myGame->getObjectDataManager()->createObjectData(type);
	if(myObjectData == NULL){
	  Logger::getLogger()->warning("Object type not found");
	  myObjectData = NULL;
	  
	}
	touchModTime();
}

void IGObject::setName(const std::string &newname){
  name = newname;
  touchModTime();
}

void IGObject::addToParent(uint32_t pid){
    parentid = pid;
    myGame->getObjectManager()->getObject(parentid)->addContainedObject(id);
    myGame->getObjectManager()->doneWithObject(parentid);
    touchModTime();
}

void IGObject::removeFromParent()
{
  myGame->getObjectManager()->getObject(parentid)->removeContainedObject(id);
    myGame->getObjectManager()->doneWithObject(parentid);
  parentid = 0;
  touchModTime();
}

int IGObject::getContainerType(){
  return myObjectData->getContainerType();
}

std::set<unsigned int> IGObject::getContainedObjects()
{
	return children;
}

bool IGObject::addContainedObject(unsigned int addObjectID)
{
  if(myObjectData->getContainerType() >= 1){
	//check that the new object is *really* inside the object
  
        //Game::getGame()->getObject(addObjectID)->parentid = id;
	touchModTime();
	return children.insert(addObjectID).second;
  }
  return false;
}

bool IGObject::removeContainedObject(unsigned int removeObjectID)
{
	// remove object
	unsigned int currsize = children.size();
	touchModTime();
	return children.erase(removeObjectID) == currsize - 1;
}

void IGObject::createFrame(Frame * frame, int playerid)
{
  
  frame->setType(ft02_Object);
  frame->packInt(id);
  frame->packInt(type);
  frame->packString(name);
  if(frame->getVersion() >= fv0_4){
    frame->packString(description);
    frame->packInt(parentid);
  }else{
    //pre tp04
    if(myObjectData != NULL){
      SizeObjectParam * size = dynamic_cast<SizeObjectParam*>(myObjectData->getParameterByType(obpT_Size));
      if(size != NULL){
       frame->packInt64(size->getSize());
      }else{
        frame->packInt64(0);
      }
    }else{
      frame->packInt64(0);
    }
    if(myObjectData != NULL){
      Position3dObjectParam * pos = dynamic_cast<Position3dObjectParam*>(myObjectData->getParameterByType(obpT_Position_3D));
      if(pos != NULL){
        pos->getPosition().pack(frame);
      }else{
        Vector3d(0,0,0).pack(frame);
      }
    }else{
      Vector3d(0,0,0).pack(frame);
    }
    if(myObjectData != NULL){
      Velocity3dObjectParam * vel = dynamic_cast<Velocity3dObjectParam*>(myObjectData->getParameterByType(obpT_Velocity));
      if(vel != NULL){
        vel->getVelocity().pack(frame);
      }else{
        Vector3d(0,0,0).pack(frame);
      }
    }else{
      Vector3d(0,0,0).pack(frame);
    }
  }
 
  std::set<unsigned int> temp = children;
  std::set < unsigned int >::iterator itcurr, itend;
  itcurr = temp.begin();
  itend = temp.end();
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
  while(itcurr != itend){
    if(!player->isVisibleObject(*itcurr)){
      std::set<unsigned int>::iterator itemp = itcurr;
      ++itcurr;
      temp.erase(itemp);
    }else{
      ++itcurr;
    }
  }

  frame->packInt(temp.size());
  //for loop for children objects
  itend = temp.end();
  for (itcurr = temp.begin(); itcurr != itend; itcurr++) {
    frame->packInt(*itcurr);
  }

  if(frame->getVersion() <= fv0_3){
  
    if(myObjectData != NULL){
      OrderQueueObjectParam* oq = dynamic_cast<OrderQueueObjectParam*>(myObjectData->getParameterByType(obpT_Order_Queue));
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
      
    }else{
      frame->packInt(0);
      frame->packInt(0);
    }
  }
  //padding
  if(frame->getVersion() >= fv0_3){
    frame->packInt64(myObjectData->getModTime());
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
    myObjectData->packObjectParameters(frame, playerid);
  }else{
    if(myObjectData != NULL){
      myObjectData->packExtraData(frame);
    }
  }

}

ObjectData* IGObject::getObjectData(){
  return myObjectData;
}

void IGObject::touchModTime(){
  if(myObjectData != NULL)
    myObjectData->touchModTime();
}

long long IGObject::getModTime() const{
  if(myObjectData != NULL)
    return myObjectData->getModTime();
  else
    return 0LL;
}

void IGObject::setParent(uint32_t pid){
    parentid = pid;
}

void IGObject::setModTime(uint64_t time){
    if(myObjectData != NULL)
        myObjectData->setModTime(time);
}

void IGObject::signalRemoval(){
  if(myObjectData != NULL)
    myObjectData->signalRemoval();
}
