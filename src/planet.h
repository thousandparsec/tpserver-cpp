#ifndef PLANET_H
#define PLANET_H

#include "ownedobject.h"

class Planet:public OwnedObject {
      public:
	Planet();

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

      private:


};

#endif
