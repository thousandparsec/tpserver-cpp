
#include "frame.h"

#include "planet.h"

Planet::Planet():OwnedObject()
{

}


void Planet::packExtraData(Frame * frame)
{
	OwnedObject::packExtraData(frame);
}

void Planet::doOnceATurn(IGObject * obj)
{

}
