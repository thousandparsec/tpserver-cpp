/*  Planet objects
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

#include "frame.h"
#include "order.h"

#include "planet.h"

Planet::Planet():OwnedObject()
{

}


void Planet::packExtraData(Frame * frame)
{
	OwnedObject::packExtraData(frame);
	frame->packInt(0); // no resources at this time
}

void Planet::doOnceATurn(IGObject * obj)
{

}

void Planet::packAllowedOrders(Frame * frame, int playerid){
  if(playerid == getOwner()){
    frame->packInt(2);
    frame->packInt(odT_Build);
    frame->packInt(odT_Nop);
  }else{
    frame->packInt(0);
  }
}

bool Planet::checkAllowedOrder(int ot, int playerid){
  return (playerid == getOwner() && (ot == odT_Build || ot == odT_Nop));
}

int Planet::getContainerType(){
  return 2;
}

ObjectData* Planet::clone(){
  return new Planet();
}

