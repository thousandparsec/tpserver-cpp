
#include "frame.h"

#include "universe.h"

Universe::Universe()
{
	yearNum = 0;
}

void Universe::packExtraData(Frame * frame)
{
	frame->packInt(yearNum);
}

void Universe::doOnceATurn(IGObject * obj)
{
	++yearNum;
}

ObjectData* Universe::clone(){
  return new Universe();
}

void Universe::setYear(int year)
{
	yearNum = year;
}

int Universe::getYear()
{
	return yearNum;
}
