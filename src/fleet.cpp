
#include "frame.h"

#include "fleet.h"

Fleet::Fleet():OwnedObject()
{
  damage = 0;
}

void Fleet::packExtraData(Frame * frame)
{
	OwnedObject::packExtraData(frame);
	
	frame->packInt(ships.size());
	for(std::map<int, int>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
	  frame->packInt(itcurr->first);
	  frame->packInt(itcurr->second);
	}
	
	frame->packInt(damage);

}

void Fleet::doOnceATurn(IGObject * obj)
{

}
