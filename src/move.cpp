/*  Move order
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"
#include "player.h"
#include "fleet.h"
#include "message.h"

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

int Move::getETA(IGObject *ob) const{
  unsigned long long distance = dest.getDistance(ob->getPosition());
  unsigned long max_speed = ((Fleet*)(ob->getObjectData()))->maxSpeed();
  
  return (int)(distance / max_speed) + 1;
}

void Move::createFrame(Frame * f, int objID, int pos)
{
	Order::createFrame(f, objID, pos);

	unsigned long long distance = dest.getDistance(Game::getGame()->getObject(objID)->getPosition());
	unsigned long max_speed = ((Fleet*)(Game::getGame()->getObject(objID)->getObjectData()))->maxSpeed();
	
	f->packInt((int)(distance / max_speed) + 1); // number of turns
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
  unsigned long long distance = dest.getDistance(ob->getPosition());
  unsigned long long max_speed = ((Fleet*)(ob->getObjectData()))->maxSpeed();

  Logger::getLogger()->debug("Moving %lld at %lld speed", distance, max_speed);

  if(distance < max_speed){
  
    ob->setFuturePosition(dest);
    
    Message * msg = new Message();
    msg->setSubject("Move order complete");
    msg->setBody("The move order is complete on this object");
    msg->setType(0);
    Game::getGame()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);

    return true;

  }else{
    
    Vector3d velo = (dest - ob->getPosition()).makeLength(max_speed);

    Logger::getLogger()->debug("velo [%lld, %lld, %lld]", velo.getX(), velo.getY(), velo.getZ());

    ob->setFuturePosition(ob->getPosition() + velo);

    return false;
  }
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
