
#include "order.h"
#include "frame.h"
#include "object.h"

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
	f->packInt(0);
	f->packInt(waitTime);
	f->packInt(1000);
}

bool Nop::inputFrame(Frame * f)
{
  f->unpackInt(); // number of turns (read only, ignore client value)
  // TODO fix below in case the client sends a list greated than zero
  f->unpackInt(); // resource list (read only, ignore client value)
	waitTime = f->unpackInt();
	
	return (waitTime >= 0);
}

bool Nop::doOrder(IGObject * ob){
  waitTime--;
  return waitTime <= 0;
}

void Nop::describeOrder(Frame * f) const
{
  Order::describeOrder(f);
  f->packString("No Operation");
  f->packString("Object does nothing for a given number of turns");
  f->packInt(1);
  f->packString("wait");
  f->packInt(opT_Time);
  f->packString("The number of turns to wait");

}

Order* Nop::clone() const{
  return new Nop();
}
