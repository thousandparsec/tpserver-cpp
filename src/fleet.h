#ifndef FLEET_H
#define FLEET_H

#include "ownedobject.h"

class Fleet:public OwnedObject {
      public:
	Fleet();

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

      private:


};

#endif
