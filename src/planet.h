#ifndef PLANET_H
#define PLANET_H

#include "ownedobject.h"

class Planet:public OwnedObject {
      public:
	Planet();

	void packExtraData(Frame * frame);

	void doOnceATurn(IGObject * obj);

	void packAllowedOrders(Frame * frame, int playerid);

	bool checkAllowedOrder(int ot, int playerid);

      private:


};

#endif
