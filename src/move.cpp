
#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"

#include "move.h"

Move::Move()
{
	type = odT_Move;
}

Move::~Move()
{

}

long long Move::getX()
{
	return x;
}

long long Move::getY()
{
	return y;
}

long long Move::getZ()
{
	return z;
}

void Move::setDest(long long x1, long long y1, long long z1)
{
	x = x1;
	y = y1;
	z = z1;
}

void Move::createFrame(Frame * f, int objID, int pos)
{
	Order::createFrame(f, objID, pos);
	f->packInt(1); // number of turns
	f->packInt(0); // size of resource list
	f->packInt64(x);
	f->packInt64(y);
	f->packInt64(z);
}

bool Move::inputFrame(Frame * f)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list
	x = f->unpackInt64();
	y = f->unpackInt64();
	z = f->unpackInt64();

	return true;
}

bool Move::doOrder(IGObject * ob){
  ob->setVelocity3(x - ob->getPositionX(), y - ob->getPositionY(), z - ob->getPositionZ());
  ob->setPosition3(x, y, z);

  ob->removeFromParent();

  // re-containerise if necessary
  std::list<unsigned int> oblist = Game::getGame()->getContainerByPos(x, y, z);
  
  for(std::list<unsigned int>::reverse_iterator itcurr = oblist.rend(); itcurr != oblist.rbegin(); --itcurr){
    if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
      Game::getGame()->getObject(*itcurr)->addContainedObject(ob->getID());
      break;
    }
  }

  return true;
}

void Move::describeOrder(int orderType, Frame * f)
{
	if (orderType == odT_Move) {
		f->packInt(odT_Move);
		f->packString("Move");
		f->packString("Move to a given position absolute in space");
		f->packInt(1);
		f->packString("pos");
		f->packInt(opT_Space_Coord_Abs);
		f->packString("The position in space to move to");
	}
}
