
#include "frame.h"

#include "order.h"

Order::Order()
{

}

Order::~Order()
{

}

OrderType Order::getType()
{
	return type;
}

void Order::setType(OrderType ot)
{
	type = ot;
}

void Order::createFrame(Frame * f, int objID, int pos)
{
	f->setType(ft_Order);
	f->packInt(objID);
	f->packInt(type);
	f->packInt(pos);

}
