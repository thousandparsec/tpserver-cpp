
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

void Order::describeOrder(int ordertype, Frame * f)
{
	if (ordertype > odT_Invalid && ordertype < odT_Max) {
		f->setType(ft_Order_Description);
		f->packInt(ordertype);
		switch (ordertype) {
		case odT_Nop:
			f->packString("No Operation");
			f->packString("Object does nothing for a given number of turns");
			f->packInt(1);
			f->packString("Time Delay");
			f->packInt(opT_Time);
			f->packString("The number of turns to wait");
			break;
		case odT_Move:
			f->packString("Move");
			f->packString("Move to a given position absolute in space");
			f->packInt(1);
			f->packString("Position");
			f->packInt(opT_Space_Coord);
			f->packString("The position in space to move to");
			break;
		default:
			f->createFailFrame(16, "Order not implemented yet");
			break;
		}
	} else {
		f->createFailFrame(15, "No such order");
	}
}
