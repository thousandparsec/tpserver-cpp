#ifndef OBJECTDATAMANAGER_H
#define OBJECTDATAMANAGER_H

#include <map>

typedef enum{
  obT_Invalid = -1,
  obT_Universe = 0,
  obT_Galaxy = 1,
  obT_Star_System = 2,
  obT_Planet = 3,
  obT_Fleet = 4,

  obT_Max
} bi_ObjectType;

class ObjectData;

class ObjectDataManager{
 public:
  ObjectDataManager();
  ~ObjectDataManager();

  ObjectData* createObjectData(int type);
  bool checkValid(int type);

  int addNewObjectType(ObjectData* od);

 private:
  std::map<int, ObjectData*> prototypeStore;
  int nextType;

};

#endif
