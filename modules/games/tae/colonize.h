#ifndef COLONIZE_H
#define COLONIZE_H
/*  Colonize class
 *  
 *  This class defines the colonize FleetOrder used by colonist fleets.
 *  The colonize order targets an uninhabited/uncolonized system which
 *  is valid for the fleet giving the order.  It creates a new colony,
 *  awards one point of the same type as the fleet to the owner of the
 *  leader in the region of the new colony.  It may also initiate an 
 *  external combat if connecting two regions with leaders of the same
 *  type.
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

#include "fleetorder.h"

class ObjectOrderParameter;

class Colonize : public FleetOrder
{
 public:
   Colonize(bool mining);
   virtual ~Colonize();

   virtual Order* clone() const;

   virtual void createFrame(OutputFrame * f, int pos);
   virtual void inputFrame(InputFrame * f, uint32_t playerid);
   
   virtual bool doOrder(IGObject::Ptr obj);

 protected:
   bool isMining;
};


#endif
