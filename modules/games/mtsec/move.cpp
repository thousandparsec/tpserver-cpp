/*  Move order
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/order.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/player.h>
#include "fleet.h"
#include <tpserver/message.h>
#include <tpserver/playermanager.h>

#include "move.h"

Move::Move() : Order()
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

	unsigned long long distance = dest.getDistance(Game::getGame()->getObjectManager()->getObject(objID)->getPosition());
	unsigned long max_speed = ((Fleet*)(Game::getGame()->getObjectManager()->getObject(objID)->getObjectData()))->maxSpeed();
        Game::getGame()->getObjectManager()->doneWithObject(objID);
	
	f->packInt((int)(distance / max_speed) + 1); // number of turns
	f->packInt(0); // size of resource list
	dest.pack(f);
	
}

bool Move::inputFrame(Frame * f, unsigned int playerid)
{
  f->unpackInt(); // number of turns
  int ressize = f->unpackInt(); // size of resource list (should be zero)
  for(int i = 0; i < ressize; i++){
    f->unpackInt(); //The resource id
    f->unpackInt(); //The amount of the resource
  }
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
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);

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
  f->packInt64(descmodtime);
}

Order* Move::clone() const{
  return new Move();
}
