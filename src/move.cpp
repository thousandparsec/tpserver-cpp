
#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"

#include "move.h"

Move::Move()
{
	type = odT_Move;
}

Move::~Move()
{

}

Vector3d Move::getDest() const
{
	return dest;
}


void Move::setDest(const Vector3d & ndest)
{
  dest = ndest;
}

void Move::createFrame(Frame * f, int objID, int pos)
{
	Order::createFrame(f, objID, pos);
	f->packInt(1); // number of turns
	f->packInt(0); // size of resource list
	dest.pack(f);
	
}

bool Move::inputFrame(Frame * f)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  dest.unpack(f);
  
  return true;
}

bool Move::doOrder(IGObject * ob){
  ob->setVelocity(dest - ob->getPosition());
  ob->setPosition(dest);

  ob->removeFromParent();

  // re-containerise if necessary
  std::list<unsigned int> oblist = Game::getGame()->getContainerByPos(dest);

  Logger::getLogger()->debug("There are %d possible container objects", oblist.size());
  
  for(std::list<unsigned int>::iterator itcurr = oblist.begin(); itcurr != oblist.end(); ++itcurr){
    Logger::getLogger()->debug("Container object %d", *itcurr);
    if(Game::getGame()->getObject(*itcurr)->getType() <= 2){
      Game::getGame()->getObject(*itcurr)->addContainedObject(ob->getID());
      break;
    }
  }

  return true;
}

void Move::describeOrder(Frame * f) const
{
  Order::describeOrder(f);
  f->packString("Move");
  f->packString("Move to a given position absolute in space");
  f->packInt(1);
  f->packString("pos");
  f->packInt(opT_Space_Coord_Abs);
  f->packString("The position in space to move to");

}

Order* Move::clone() const{
  return new Move();
}
