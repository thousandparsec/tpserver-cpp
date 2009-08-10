#ifndef REINFORCE_H
#define REINFORCE_H
/*  Reinforce order used in combat by colonist fleets. It adds one to
 *  the owner's strength for that round of combat.
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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

class Frame;
class ObjectOrderParameter;


class Reinforce : public Order
{
 public:
   Reinforce();
   virtual ~Reinforce();

   virtual Order* clone() const;

   virtual void createFrame(Frame * f, int pos);
   virtual void inputFrame(InputFrame * f, uint32_t playerid);
   
   virtual bool doOrder(IGObject::Ptr obj);
};


#endif
