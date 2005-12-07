/*  In Game Objects in the game universe
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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
#include "player.h"

#include "object.h"

Game *IGObject::myGame = NULL;

IGObject::IGObject()
{
	id = 0xffffffff;
	parentid = 0;
	name = NULL;
	if (myGame == NULL) {
		myGame = Game::getGame();
	}
	myObjectData = NULL;
    ordernum = 0;
}

IGObject::IGObject(IGObject & rhs)
{
	if (rhs.name != NULL) {
		int len = strlen(rhs.name) + 1;
		if (name != NULL)
			delete[]name;
		name = new char[len];
		strncpy(name, rhs.name, len);
	} else {
		name = NULL;
	}
	Logger::getLogger()->warning("Object Copy Constructor: copying Object ID");
	id = rhs.id;
	type = rhs.type;
	size = rhs.size;
	pos = rhs.pos;
	vel = rhs.vel;

	parentid = rhs.parentid;

	children.clear();
	//children = rhs.children;
	touchModTime();
}

IGObject::~IGObject()
{
	children.clear();
	if (name != NULL) {
		delete[]name;
	}
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

unsigned long long IGObject::getSize()
{
	return size;
}

char *IGObject::getName()
{
	char *rtn = NULL;
	if (name != NULL) {
		int len = strlen(name) + 1;
		rtn = new char[len];
		strncpy(rtn, name, len);
	}
	return rtn;
}

Vector3d IGObject::getPosition()
{
	return pos;
}

Vector3d IGObject::getVelocity()
{
	return vel;
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

void IGObject::setSize(unsigned long long newsize)
{
	size = newsize;
	touchModTime();
}

void IGObject::setName(char *newname)
{
	if (name != NULL) {
		delete[]name;
	}
	int len = strlen(newname) + 1;
	name = new char[len];
	strncpy(name, newname, len);
	touchModTime();
}

void IGObject::setPosition(const Vector3d & npos)
{
	pos = npos;
	futurepos = npos;
	touchModTime();
}

void IGObject::setFuturePosition(const Vector3d & npos){
  futurepos = npos;
}

void IGObject::updatePosition(){
    Vector3d nvel = futurepos - pos;
    if(nvel != vel){
        vel = nvel;
        touchModTime();
    }

  // recontainerise if necessary
  if(pos != futurepos && myGame->getObjectManager()->getObject(parentid)->getContainerType() >= 1){
    //removeFromParent();
    myGame->getObjectManager()->doneWithObject(parentid);
    std::set<unsigned int> oblist = myGame->getObjectManager()->getContainerByPos(futurepos);
    for(std::set<unsigned int>::reverse_iterator itcurr = oblist.rbegin(); itcurr != oblist.rend(); ++itcurr){
      Logger::getLogger()->debug("Container object %d", *itcurr);
      //if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
      if(*itcurr != id && myGame->getObjectManager()->getObject(*itcurr)->size >= size){
        if(*itcurr != parentid){
            removeFromParent();
            addToParent(*itcurr);
        }
	break;
      }
        myGame->getObjectManager()->doneWithObject(*itcurr);
    }
    if(parentid == 0){
        removeFromParent();
        addToParent(0);
    }
    pos = futurepos;
    touchModTime();
  }else{
    myGame->getObjectManager()->doneWithObject(parentid);
    }
}

void IGObject::setVelocity(const Vector3d & nvel)
{
	vel = nvel;
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

uint32_t IGObject::getNumOrders(int playerid)
{
    //check if nop order (order type 0) is allowed
  if(myObjectData->checkAllowedOrder(0, playerid)){
    return ordernum;
  }
  return 0;
}

void IGObject::setNumOrders(uint32_t num){
    touchModTime();
    ordernum = num;
    Logger::getLogger()->debug("IGObject::setNumOrders");
}

void IGObject::createFrame(Frame * frame, int playerid)
{
  
  frame->setType(ft02_Object);
  frame->packInt(id);
  frame->packInt(type);
  frame->packString(name);
  frame->packInt64(size);
  pos.pack(frame);
  vel.pack(frame);
 
  std::set<unsigned int> temp = children;
  std::set < unsigned int >::iterator itcurr, itend;
  itcurr = temp.begin();
  itend = temp.end();
  Player* player = Game::getGame()->getPlayer(playerid);
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

  int templength = frame->getDataLength();

  myObjectData->packAllowedOrders(frame, playerid);

  if(frame->getDataLength() - templength > 4){
    frame->packInt(ordernum);
  }else{
    frame->packInt(0);
  }

  
  if(frame->getVersion() > fv0_1){
    if(frame->getVersion() < fv0_3){
      frame->packInt(0);
      frame->packInt(0);
    }else{
      frame->packInt64(myObjectData->getModTime());
    }
    frame->packInt(0);
    frame->packInt(0);
  }

  if(myObjectData != NULL){
    myObjectData->packExtraData(frame);
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
