#include "string.h"

#include "frame.h"
#include "logging.h"
#include "game.h"

#include "object.h"

Game* IGObject::myGame = NULL;

unsigned int IGObject::nextAutoID = 0;

IGObject::IGObject(){
  id = nextAutoID++;
  //check that new id is valid - TODO
  name = NULL;
  if(myGame == NULL){
    myGame = Game::getGame();
  }
}

IGObject::IGObject(IGObject& rhs){
  if(rhs.name != NULL){
    int len = strlen(rhs.name) + 1;
    if(name != NULL)
      delete[] name;
    name = new char[len];
    strncpy(name, rhs.name, len);
  }else{
    name = NULL;
  }
  Logger::getLogger()->warning("Object Copy Constructor: copying Object ID");
  id = rhs.id;
  type = rhs.type;
  size = rhs.size;
  posx = rhs.posx;
  posy = rhs.posy;
  posz = rhs.posz;
  velx = rhs.velx;
  vely = rhs.vely;
  velz = rhs.velz;
  accx = rhs.accx;
  accy = rhs.accy;
  accz = rhs.accz;
  children.clear();
  children = rhs.children;
}

IGObject::~IGObject(){
  children.clear();
  if(name != NULL){
    delete[] name;
  }
}

IGObject IGObject::operator=(IGObject& rhs){
  //TODO
  return *this;
}

unsigned int IGObject::getID(){
  return id;
}

unsigned int IGObject::getType(){
  return type;
}

unsigned long long IGObject::getSize(){
  return size;
}

char* IGObject::getName(){
  char* rtn = NULL;
  if(name != NULL){
    int len = strlen(name) + 1;
    rtn = new char[len];
    strncpy(rtn, name, len);
  }
  return rtn;
}

long long IGObject::getPositionX(){
  return posx;
}

long long IGObject::getPositionY(){
  return posy;
}

long long IGObject::getPositionZ(){
  return posz;
}

long long IGObject::getVelocityX(){
  return velx;
}

long long IGObject::getVelocityY(){
  return vely;
}

long long IGObject::getVelocityZ(){
  return velz;
}

long long IGObject::getAccelerationX(){
  return accx;
}

long long IGObject::getAccelerationY(){
  return accy;
}

long long IGObject::getAccelerationZ(){
  return accz;
}

std::list<IGObject*> IGObject::getContainedObjects(){
  return children;
}

bool IGObject::setID(unsigned int newid){
  //check to see if id is use
  //if it is, then return false
  //else
  id = newid;
  return true;
}

void IGObject::autoSetID(){
  id = nextAutoID++;
  //check that id is valid
}

void IGObject::setType(unsigned int newtype){
  type = newtype;
}

void IGObject::setSize(unsigned long long newsize){
  size = newsize;
}

void IGObject::setName(char* newname){
  if(name != NULL){
    delete[] name;
  }
  int len = strlen(newname) + 1;
  name = new char[len];
  strncpy(name, newname, len);
}

void IGObject::setPosition3(long long x, long long y, long long z){
  posx = x;
  posy = y;
  posz = z;
}

void IGObject::setVelocity3(long long x, long long y, long long z){
  velx = x;
  vely = y;
  velz = z;
}

void IGObject::setAcceleration3(long long x, long long y, long long z){
  accx = x;
  accy = y;
  accz = z;
}

bool IGObject::addContainedObject(IGObject* addObject){
  //check that the new object is *really* inside the object

  return true;
}

bool IGObject::removeContainedObject(IGObject* removeObject){
  // remove object

  return true;
}

void IGObject::createFrame(Frame* frame){
  frame->setType(ft_Object);
  frame->packInt(id);
  frame->packInt(type);
  frame->packString(name);
  frame->packInt64(size);
  frame->packInt64(posx);
  frame->packInt64(posy);
  frame->packInt64(posz);
  frame->packInt64(velx);
  frame->packInt64(vely);
  frame->packInt64(velz);
  frame->packInt64(accx);
  frame->packInt64(accy);
  frame->packInt64(accz);
  //frame->packInt(children.size());
  frame->packInt(0); //HACK hack
  //for loop for children objects


}
