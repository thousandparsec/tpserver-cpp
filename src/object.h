#ifndef OBJECT_H
#define OBJECT_H

#include <list>

class Frame;
class Game;

class IGObject{

 public:
  IGObject();
  IGObject(IGObject& rhs);
  ~IGObject();

  IGObject operator=(IGObject& rhs);

  unsigned int getID();
  unsigned int getType();
  unsigned long long getSize();
  char* getName();
  long long getPositionX();
  long long getPositionY();
  long long getPositionZ();
  long long getVelocityX();
  long long getVelocityY();
  long long getVelocityZ();
  long long getAccelerationX();
  long long getAccelerationY();
  long long getAccelerationZ();
  std::list<IGObject*> getContainedObjects();

  bool setID(unsigned int newid);
  void autoSetID();
  void setType(unsigned int newtype);
  void setSize(unsigned long long newsize);
  void setName(char* newname);
  void setPosition3(long long x, long long y, long long z);
  void setVelocity3(long long x, long long y, long long z);
  void setAcceleration3(long long x, long long y, long long z);
  
  bool addContainedObject(IGObject* addObject);
  bool removeContainedObject(IGObject* removeObject);
  
  void createFrame(Frame* frame);
  
 protected:
  static Game* myGame;

 private:
  
  static unsigned int nextAutoID;
  
  unsigned int id;
  unsigned int type;
  unsigned long long size;
  char* name;
  long long posx;
  long long posy;
  long long posz;
  long long velx;
  long long vely;
  long long velz;
  long long accx;
  long long accy;
  long long accz;
  
  std::list<IGObject*> children;

};

#endif
