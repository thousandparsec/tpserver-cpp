#ifndef FLEET_H
#define FLEET_H

#include <map>

#include "ownedobject.h"

class Fleet:public OwnedObject {
      public:
	Fleet();

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

      private:
	std::map<int, int> ships;
	int damage;

};

#endif
