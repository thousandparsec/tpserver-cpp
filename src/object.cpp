#include <string.h>
#include <cassert>

#include "frame.h"
#include "logging.h"
#include "game.h"
#include "order.h"
#include "objectdata.h"
#include "emptyobject.h"
#include "universe.h"
#include "planet.h"
#include "fleet.h"

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

	switch(type){
	case 0:
	  myObjectData = new Universe();
	  break;
	case 1:
	case 2:
	  myObjectData = new EmptyObject();
	  break;
	case 3:
	  myObjectData = new Planet();
	  break;
	case 4:
	  myObjectData = new Fleet();
	  break;
	default:
	  Logger::getLogger()->warning("Object type not found");
	  myObjectData = NULL;
	  break;
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
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(playerid);
	if (ordit != actions.end()) {
		if ((*ordit).second.find(ord->getType()) != (*ordit).second.end()) {
			if (pos == -1) {
				orders.push_back(ord);
			} else {
				std::list < Order * >::iterator inspos = orders.begin();
				advance(inspos, pos);
				orders.insert(inspos, ord);
			}
			return true;
		}
	}
	return false;
}

bool IGObject::removeOrder(int pos, int playerid)
{
        if (pos >= orders.size()) {
                return false;
        }
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(playerid);
	if (ordit != actions.end()) {
		std::list < Order * >::iterator delpos = orders.begin();
		assert (pos < orders.size());
		advance(delpos, pos);
		delete(*delpos);
		orders.erase(delpos);
		return true;
	}
	return false;
}

Order *IGObject::getOrder(int pos, int playerid)
{
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(playerid);
	if (ordit != actions.end()) {
		std::list < Order * >::iterator showpos = orders.begin();
		advance(showpos, pos);
		if (showpos != orders.end()) {
			return (*showpos);
		}
	}
	return NULL;
}

int IGObject::getNumOrders(int playerid)
{
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(playerid);
	if (ordit != actions.end()) {
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

bool IGObject::addAction(int currpid, int newpid, OrderType ot)
{
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(currpid);
	if (ordit != actions.end() || currpid == -1) {
		actions[newpid].insert(ot);
		return true;
	}
	return false;
}

bool IGObject::removeAction(int currpid, int newpid, OrderType ot)
{
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(currpid);
	if (ordit != actions.end() || currpid == -1) {
		actions[newpid].erase(ot);
		if (actions[newpid].empty()) {
			actions.erase(newpid);
		}
		return true;
	}
	return false;
}

std::set<OrderType> IGObject::getActions(int currpid, int newpid)
{
	std::map < int, std::set < OrderType > >::iterator ordit = actions.find(currpid);
	if (ordit != actions.end()) {
		std::map < int, std::set < OrderType > >::iterator ordit2 = actions.find(newpid);
		if (ordit2 != actions.end()) {
			return (*ordit2).second;
		}
	}
	return std::set < OrderType > ();
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
  //frame->packInt(0); //HACK hack
  //for loop for children objects
  std::set < unsigned int >::iterator itcurr, itend;
  itend = children.end();
  for (itcurr = children.begin(); itcurr != itend; itcurr++) {
    frame->packInt(*itcurr);
  }
  std::map < int, std::set < OrderType > >::iterator ord = actions.find(playerid);
  if (ord != actions.end()) {
    frame->packInt(((*ord).second).size());
    std::set < OrderType >::iterator itacurr, itaend;
    itaend = ((*ord).second).end();
    for (itacurr = ((*ord).second).begin(); itacurr != itaend; itacurr++) {
      frame->packInt(*itacurr);
    }
  } else {
    frame->packInt(0);
  }
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
