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
	int numShips(int type);

	long long maxSpeed();
	int firepower(bool draw);
	bool hit(int firepower);

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

	void packAllowedOrders(Frame * frame, int playerid);

	bool checkAllowedOrder(int ot, int playerid);

	int getContainerType();

	ObjectData* clone();

      private:
	std::map<int, int> ships;
	int damage;

};

#endif
