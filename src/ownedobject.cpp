
#include "frame.h"

#include "ownedobject.h"

OwnedObject::OwnedObject()
{
	playerid = -1;
}

void OwnedObject::packExtraData(Frame * frame)
{
	frame->packInt(playerid);
}

void OwnedObject::setOwner(int player)
{
	playerid = player;
}

int OwnedObject::getOwner()
{
	return playerid;
}
