#include <stdlib.h>

#include "frame.h"

#include "order.h"

/*Order::Order()
{

}

Order::~Order()
{

}
*/

int Order::getType() const
{
	return type;
}

void Order::setType(int ntype){
  type = ntype;
}

void Order::createFrame(Frame * f, int objID, int pos)
{

  f->setType(ft02_Order);
  f->packInt(objID);
  f->packInt(pos);
  f->packInt(type);
  
}

bool Order::inputFrame(Frame * f)
{
  return true;
}


void Order::describeOrder(Frame * f) const
{
  f->setType(ft02_OrderDesc);
  f->packInt(type);
}


