

#include "emptyobject.h"

void EmptyObject::packExtraData(Frame * frame)
{

}

void EmptyObject::doOnceATurn(IGObject * obj)
{

}

ObjectData* EmptyObject::clone(){
  return new EmptyObject();
}
