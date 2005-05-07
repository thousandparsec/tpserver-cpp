/*  Nop order
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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
#include "player.h"
#include "message.h"
#include "ownedobject.h"

#include "nop.h"

Nop::Nop() : Order()
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
  if(waitTime <= 0){
    
    Message * msg = new Message();
    msg->setSubject("NOp order complete");
    msg->setBody("The object has finished it's delay and is now continuing");
    Game::getGame()->getPlayer(((OwnedObject*)(ob->getObjectData()))->getOwner())->postToBoard(msg);

    return true;
  }else{
    return false;
  }
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
  f->packInt64(descmodtime);
}

Order* Nop::clone() const{
  return new Nop();
}
