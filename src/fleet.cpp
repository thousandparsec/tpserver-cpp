
#include "frame.h"

#include "fleet.h"

Fleet::Fleet():OwnedObject()
{

}

void Fleet::packExtraData(Frame * frame)
{
	OwnedObject::packExtraData(frame);
}

void Fleet::doOnceATurn(IGObject * obj)
{

}
