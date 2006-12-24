/*  Order baseclass
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

#include <stdlib.h>
#include <time.h>

#include "frame.h"

#include "order.h"

Order::Order()
{
  descmodtime = time(NULL);
}

Order::~Order()
{

}

int Order::getType() const
{
	return type;
}

void Order::setType(int ntype){
  type = ntype;
}

void Order::createFrame(Frame * f, int objID, int pos)
{

  f->setType(ft02_Order);
  f->packInt(objID);
  f->packInt(pos);
  f->packInt(type);
  
}

bool Order::inputFrame(Frame * f, unsigned int playerid)
{
  return true;
}


void Order::describeOrder(Frame * f) const
{
  f->setType(ft02_OrderDesc);
  f->packInt(type);
}

uint64_t Order::getDescriptionModTime() const{
  return descmodtime;
}
