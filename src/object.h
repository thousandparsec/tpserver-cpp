#ifndef OBJECT_H
#define OBJECT_H

#include <list>

class Object{

 public:
  Object();
  Object(Object& rhs);
  ~Object();

  Object operator=(Object& rhs);

  

 private:
  list<Result> result;

};

#endif
