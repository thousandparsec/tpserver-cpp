
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

int Universe::getContainerType(){
  return 1;
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
