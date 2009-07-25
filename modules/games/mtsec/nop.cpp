/*  Nop order
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <tpserver/game.h>
#include <tpserver/player.h>
#include <tpserver/message.h>
#include "ownedobject.h"
#include <tpserver/playermanager.h>
#include <tpserver/timeparameter.h>
#include <tpserver/logging.h>

#include "nop.h"

namespace MTSecRuleset {

Nop::Nop() : Order()
{
  name = "No Operation";
  description = "Object does nothing for a given number of turns";
  
  timeparam = (TimeParameter*)addOrderParameter( new TimeParameter("wait","The number of turns to wait") );
}

Nop::~Nop(){
}

void Nop::createFrame(Frame * f, int pos)
{
  turns = timeparam->getTime();
  Order::createFrame(f, pos);
}

Result Nop::inputFrame(Frame * f, uint32_t playerid)
{
  Result rtv = Order::inputFrame(f, playerid);
  turns = timeparam->getTime();

  return rtv;
}

bool Nop::doOrder(IGObject * ob){
  if(timeparam->getTime() <= 1){
    
    Message * msg = new Message();
    msg->setSubject("NOp order complete");
    msg->setBody("The object has finished it's delay and is now continuing");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
    OwnedObject* ownedobject = dynamic_cast<OwnedObject*>(    ob->getObjectBehaviour());
    if(ownedobject != NULL){
      Game::getGame()->getPlayerManager()->getPlayer(ownedobject->getOwner())->postToBoard(msg);
    }else{
      Logger::getLogger()->debug("Nop order not on Owned Object");
      delete msg;
    }

    return true;
  }else{
    turns--;
    timeparam->setTime(timeparam->getTime() - 1);
    return false;
  }
}

Order* Nop::clone() const{
  Nop* nn = new Nop();
  nn->type = type;
  return nn;
}

}

