
#include "frame.h"
#include "order.h"

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

void Planet::packAllowedOrders(Frame * frame, int playerid){
  if(playerid == getOwner()){
    frame->packInt(2);
    frame->packInt(odT_Build);
    frame->packInt(odT_Nop);
  }else{
    frame->packInt(0);
  }
}

bool Planet::checkAllowedOrder(int ot, int playerid){
  return (playerid == getOwner() && (ot == odT_Build || ot == odT_Nop));
}
