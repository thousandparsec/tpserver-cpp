

#include "frame.h"
#include "order.h"

#include "objectdata.h"

void ObjectData::packAllowedOrders(Frame * frame, int playerid){
  frame->packInt(0);
}

bool ObjectData::checkAllowedOrder(int ot, int playerid){
  return false;
}
