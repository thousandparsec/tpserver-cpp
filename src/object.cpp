#include <string.h>
#include <cassert>

#include "frame.h"
#include "logging.h"
#include "game.h"
#include "order.h"
#include "objectdata.h"
#include "objectdatamanager.h"

#include "object.h"

Game *IGObject::myGame = NULL;

unsigned int IGObject::nextAutoID = 0;

IGObject::IGObject()
{
	id = nextAutoID++;
	parentid = 0;
	//check that new id is valid - TODO
	name = NULL;
	if (myGame == NULL) {
		myGame = Game::getGame();
	}
	myObjectData = NULL;
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


std::set<unsigned int> IGObject::getContainedObjects()
{
	return children;
}

bool IGObject::setID(unsigned int newid)
{
	//check to see if id is use
	//if it is, then return false
	//else
	id = newid;
	return true;
}

unsigned int IGObject::getParent(){
  return parentid;
}

void IGObject::autoSetID()
{
	id = nextAutoID++;
	//check that id is valid
}

void IGObject::setType(unsigned int newtype)
{
	type = newtype;

	if(myObjectData != NULL)
	  delete myObjectData;

	myObjectData = Game::getGame()->getObjectDataManager()->createObjectData(type);
	if(myObjectData == NULL){
	  Logger::getLogger()->warning("Object type not found");
	  myObjectData = NULL;
	  
	}
}

void IGObject::setSize(unsigned long long newsize)
{
	size = newsize;
}

void IGObject::setName(char *newname)
{
	if (name != NULL) {
		delete[]name;
	}
	int len = strlen(newname) + 1;
	name = new char[len];
	strncpy(name, newname, len);
}

void IGObject::setPosition(const Vector3d & npos)
{
	pos = npos;
}

void IGObject::setVelocity(const Vector3d & nvel)
{
	vel = nvel;
}

void IGObject::removeFromParent()
{
  Game::getGame()->getObject(parentid)->removeContainedObject(id);
  parentid = 0;
}

bool IGObject::addContainedObject(unsigned int addObjectID)
{
	//check that the new object is *really* inside the object
  
        Game::getGame()->getObject(addObjectID)->parentid = id;

	return children.insert(addObjectID).second;
}

bool IGObject::removeContainedObject(unsigned int removeObjectID)
{
	// remove object
	unsigned int currsize = children.size();
	return children.erase(removeObjectID) == currsize - 1;
}

bool IGObject::addOrder(Order * ord, int pos, int playerid)
{
  if (myObjectData->checkAllowedOrder(ord->getType(), playerid)) {
    if (pos == -1) {
      orders.push_back(ord);
    } else {
      std::list < Order * >::iterator inspos = orders.begin();
      advance(inspos, pos);
      orders.insert(inspos, ord);
    }
    return true;
  }
	
  return false;
}

bool IGObject::removeOrder(unsigned int pos, int playerid)
{
        if (pos >= orders.size()) {
                return false;
        }

	std::list < Order * >::iterator delpos = orders.begin();
	assert (pos < orders.size());
	advance(delpos, pos);
	if(myObjectData->checkAllowedOrder((*delpos)->getType(), playerid)){
	  delete(*delpos);
	  orders.erase(delpos);
	  return true;
	}
	return false;
}

Order *IGObject::getOrder(int pos, int playerid)
{

  std::list < Order * >::iterator showpos = orders.begin();
  advance(showpos, pos);
  if (showpos != orders.end() && myObjectData->checkAllowedOrder((*showpos)->getType(), playerid)) {
    return (*showpos);
  }
  return NULL;
}

int IGObject::getNumOrders(int playerid)
{
  if((!orders.empty()) && myObjectData->checkAllowedOrder(orders.front()->getType(), playerid)){
    return orders.size();
  }
  return 0;
}

Order * IGObject::getFirstOrder(){
  if(orders.empty())
    return NULL;
  return orders.front();
}

void IGObject::removeFirstOrder(){
  delete orders.front();
  orders.pop_front();
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
 
  frame->packInt(children.size());
  //for loop for children objects
  std::set < unsigned int >::iterator itcurr, itend;
  itend = children.end();
  for (itcurr = children.begin(); itcurr != itend; itcurr++) {
    frame->packInt(*itcurr);
  }

  myObjectData->packAllowedOrders(frame, playerid);
  // think about the next one...
  frame->packInt(orders.size());

  if(myObjectData != NULL){
    myObjectData->packExtraData(frame);
  }
  if(frame->getVersion() > fv0_1){
    frame->packInt(0);
    frame->packInt(0);
    frame->packInt(0);
    frame->packInt(0);
  }
}

ObjectData* IGObject::getObjectData(){
  return myObjectData;
}
