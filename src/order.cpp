#include <stdlib.h>

#include "frame.h"
#include "nop.h"
#include "move.h"

#include "order.h"

/*Order::Order()
{

}

Order::~Order()
{

}
*/

OrderType Order::getType()
{
	return type;
}

void Order::createFrame(Frame * f, int objID, int pos)
{
  if(f->getVersion() == fv0_1)
	f->setType(ft_Order);
  else
    f->setType(ft02_Order);
	f->packInt(objID);
	f->packInt(type);
	f->packInt(pos);

}

void Order::inputFrame(Frame * f)
{
}

void Order::createOutcome(Frame * f, int objID, int pos)
{
	f->setType(ft_Outcome);
	f->packInt(objID);
	f->packInt(pos);
}

void Order::describeOrder(int ordertype, Frame * f)
{
	if (ordertype > odT_Invalid && ordertype < odT_Max) {
	  if(f->getVersion() == fv0_1)
		f->setType(ft_Order_Description);
	  else
	    f->setType(ft02_OrderDesc);
		switch (ordertype) {
		case odT_Nop:
			Nop::describeOrder(ordertype, f);
			break;
		case odT_Move:
			Move::describeOrder(ordertype, f);
			break;
		default:
			f->createFailFrame(fec_NonExistant, "Order not implemented yet");
			break;
		}
	} else {
		f->createFailFrame(fec_NonExistant, "No such order");
	}
}

Order *Order::createOrder(OrderType ordertype)
{
	Order *rtv = NULL;
	if (ordertype > odT_Invalid && ordertype < odT_Max) {
		switch (ordertype) {
		case odT_Nop:
			rtv = new Nop();
			break;
		case odT_Move:
			rtv = new Move();
			break;
		}
	}
	return rtv;
}
