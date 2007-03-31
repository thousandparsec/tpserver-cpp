/*  Move order
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/result.h>
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
#include <tpserver/spacecoordparam.h>

#include "move.h"

Move::Move() : Order()
{
  name = "Move";
  description = "Move to a given position absolute in space";
  
  coords = new SpaceCoordParam();
  coords->setName("pos");
  coords->setDescription("The position in space to move to");
  parameters.push_back(coords);
}

Move::~Move()
{
  delete coords;
}

Vector3d Move::getDest() const
{
	return coords->getPosition();
}


void Move::setDest(const Vector3d & ndest)
{
  coords->setPosition(ndest);
}

int Move::getETA(IGObject *ob) const{
  unsigned long long distance = coords->getPosition().getDistance(ob->getPosition());
  unsigned long max_speed = ((Fleet*)(ob->getObjectData()))->maxSpeed();
  
  if(distance == 0) 
    return 1;
  return (int)((distance - 1) / max_speed) + 1;
}

void Move::createFrame(Frame * f, int objID, int pos)
{
  turns = getETA(Game::getGame()->getObjectManager()->getObject(objID));
  Game::getGame()->getObjectManager()->doneWithObject(objID);
  
  Order::createFrame(f, objID, pos);	
}

Result Move::inputFrame(Frame * f, unsigned int playerid)
{
  return Order::inputFrame(f, playerid);
}

bool Move::doOrder(IGObject * ob){
  Vector3d dest = coords->getPosition();
  unsigned long long distance = dest.getDistance(ob->getPosition());
  unsigned long long max_speed = ((Fleet*)(ob->getObjectData()))->maxSpeed();

  Logger::getLogger()->debug("Object(%d)->Move->doOrder(): Moving %lld at %lld speed (will take about %lld turns)", 
	ob->getID(), distance, max_speed, distance/max_speed);
  if(distance <= max_speed){
    Logger::getLogger()->debug("Object(%d)->Move->doOrder(): Is arriving at [%lld, %lld, %lld] ", 
      ob->getID(), dest.getX(), dest.getY(), dest.getZ());
  
    ob->setFuturePosition(dest, true);
    
    Message * msg = new Message();
    msg->setSubject("Move order complete");
    msg->setBody("The move order is complete on this object");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    Game::getGame()->getPlayerManager()->getPlayer(((Fleet*)(ob->getObjectData()))->getOwner())->postToBoard(msg);

    return true;

  }else{
    Vector3d velo = (dest - ob->getPosition()).makeLength(max_speed);
    Vector3d arriveat = ob->getPosition()+velo;
    Logger::getLogger()->debug("Move->doOrder(%d): Velocity is [%lld, %lld, %lld] (will arrive at [%lld, %lld, %lld])", 
      ob->getID(), velo.getX(), velo.getY(), velo.getZ(), arriveat.getX(), arriveat.getY(), arriveat.getZ());

    ob->setFuturePosition(arriveat);

    return false;
  }
}

Order* Move::clone() const{
  Move *nm = new Move();
  nm->type = type;
  return nm;
}
