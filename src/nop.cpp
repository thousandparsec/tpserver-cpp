
#include "order.h"
#include "frame.h"

#include "nop.h"

Nop::Nop()
{
	type = odT_Nop;
}

Nop::~Nop()
{

}

int Nop::getTime()
{
	return waitTime;
}

void Nop::setTime(int time)
{
	waitTime = time;
}

void Nop::createFrame(Frame * f, int objID, int pos)
{
	Order::createFrame(f, objID, pos);
	f->packInt(waitTime);
}

void Nop::inputFrame(Frame * f)
{
	waitTime = f->unpackInt();
}

void Nop::createOutcome(Frame * f, int objID, int pos)
{
	Order::createOutcome(f, objID, pos);
	f->packInt(waitTime);
}

void Nop::describeOrder(int orderType, Frame * f)
{
	if (orderType == odT_Nop) {
		f->packInt(odT_Nop);
		f->packString("No Operation");
		f->packString("Object does nothing for a given number of turns");
		f->packInt(1);
		f->packString("Time Delay");
		f->packInt(opT_Time);
		f->packString("The number of turns to wait");
	}
}
