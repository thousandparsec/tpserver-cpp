#ifndef EMPTYOBJECT_H
#define EMPTYOBJECT_H

#include "objectdata.h"

class EmptyObject:public ObjectData {
      public:
	void packExtraData(Frame * frame);
	void doOnceATurn(IGObject * obj);
	int getContainerType();

	ObjectData* clone();

};

#endif
