#ifndef FLEET_H
#define FLEET_H

#include <map>

#include "ownedobject.h"

class Fleet:public OwnedObject {
      public:
	Fleet();
	virtual ~Fleet();

	void addShips(int type, int number);
	bool removeShips(int type, int number);

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

      private:
	std::map<int, int> ships;
	int damage;

};

#endif
