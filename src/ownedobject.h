#ifndef OWNEDOBJECT_H
#define OWNEDOBJECT_H

#include "objectdata.h"

class OwnedObject:public ObjectData {
      public:
	OwnedObject();

	void packExtraData(Frame * frame);

	void setOwner(int player);
	int getOwner();

      private:
	int playerid;

};

#endif
